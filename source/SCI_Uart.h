//====================================================================
//SCI���ڲ�������
//====================================================================

#include "DSP28x_Project.h"

#define SYSCLK 150000000L							//30MHz
#define SCIBaudRate 9600L
#define TIMEROUTSCI (Uint32)5*(SYSCLK/SCIBaudRate)	//����ĵȴ���ʱʱ�䣬�����ʵ���޸�

#define SCI_FIFO_LEN  1 	//����DSP����FIFO���
#define UartRxLEN 20  		//���ջ��泤��
#define UartTxLEN 20  		//���ͻ��泤��

#define RTU_TIMEROUT 5 		//ms ���ڵȴ����ڵ������ַ��Ͳ�������ĳ�ʱʱ��

typedef struct Uart_Type{
	union
	{
		Uint16 All;
	    struct{
			Uint16  UartRevFlag		:1;		//���յ����ݱ�־
			Uint16  HWOVFlag		:1;		//DSPӲ���������������־

			Uint16  rFifoDataflag	:1;		//�����ڴ�ǿ�
			Uint16  rFifoFullflag	:1;		//�����ڴ����

			Uint16  DISRevflag		:1;		//���չر�

		}Status_Bits;
	}Mark_Para;

	char rxData[UartRxLEN];					//���ջ���
	Uint16 rxReadIndex;						//����FIFOд������
	Uint16 rxWriteIndex;					//����FIFO��������

	Uint16 timerOut;						//��ʱ�ж�
}Uart_Msg; 

Uart_Msg SCI_Msg;

//ModbusЭ�鷽ʽ,����Э�鶨��
char send_data[16]={0x55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x0d};			//���͵�����
//���͵����ݸ���TFT LCD�ı�����
//EE B1 10 Screen_id Control_id  	Strings    		FF FC FF FF
//EE B1 10   00 01     00 01     32 33 2E 30 35     FF FC FF FF =23.05
char send_Tdata[16]={0xEE,0xB1,0x10,0x00,0x01,0x00,0,0,0,0,0,0,0xFF,0xFC,0xFF,0xFF};

//��ȡ��������
char send_Rdata[11]={0xEE,0xB1,0x11,0x00,0x02,0x00,0x07,0xFF,0xFC,0xFF,0xFF};

//--------------------------------------------------------------------
void scic_init(void);
void scic_xmit(int a);
void send_msg(char s[]);											//��������ֵ
void send_msgc(char *str,int len);									//�����ַ�ֵ

//---------------------------------------------------------------------
//���ڽ����жϺ���
//����FIFO���ƣ����棩
//SCI_FIFO_LEN ����Ϊ 1�����Ϊ4
//---------------------------------------------------------------------
interrupt void uartRx_isr(void)
{
	if(ScicRegs.SCIFFRX.bit.RXFFOVF == 0)			//����FIFOδ���
	{
		SCI_Msg.Mark_Para.Status_Bits.rFifoDataflag = 1;

		if((SCI_Msg.rxWriteIndex + SCI_FIFO_LEN) != SCI_Msg.rxReadIndex )
		{
			//��������
			while(ScicRegs.SCIFFRX.bit.RXFFST)
			{
				SCI_Msg.rxData[SCI_Msg.rxWriteIndex] = ScicRegs.SCIRXBUF.all;
				SCI_Msg.rxWriteIndex=(++SCI_Msg.rxWriteIndex)%(UartRxLEN);
			}
		}
		else
		{
			//�û��������������Ĵ���,
			ScicRegs.SCIFFRX.bit.RXFIFORESET = 0;  	//Write 0 to reset the FIFO pointer to zero, and hold in reset.
			ScicRegs.SCIFFRX.bit.RXFIFORESET = 1 ; 	//Re-enable receive FIFO operation

			SCI_Msg.Mark_Para.Status_Bits.rFifoFullflag = 1;
		}
	}
	else
	{
		//�û�����������Ӳ������Ĵ���,������ȫ��ȡ��FIFO������ݻ������FIFO
		//�������FIFO����
		ScicRegs.SCIFFRX.bit.RXFFOVRCLR=1;   		// Clear HW Overflow flag
		ScicRegs.SCIFFRX.bit.RXFIFORESET = 0;  		// Write 0 to reset the FIFO pointer to zero, and hold in reset.
		ScicRegs.SCIFFRX.bit.RXFIFORESET = 1 ; 		// Re-enable receive FIFO operation
		SCI_Msg.Mark_Para.Status_Bits.HWOVFlag = 1;
	}
    ScicRegs.SCIFFRX.bit.RXFFINTCLR=1;   			// Clear Interrupt flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;
}

//---------------------------------------------------------------------
//���ڳ�ʼ��
//9600  8N1
//---------------------------------------------------------------------
void scic_init()
{
	InitScicGpio();

    ScicRegs.SCICTL1.bit.SWRESET =0;
 	ScicRegs.SCICCR.all =0x0007;   					// 1 stop bit,  No loopback,No parity,8 char bits, async mode, idle-line protocol

	//��ϵͳ�����CPU_FRQ_150MHZ=1
	//BaudRate=LSPCLK/[(BRR+1)*8]
	#if (CPU_FRQ_150MHZ)							// 150 MHz CPU Freq (30 MHz input freq) by DEFAULT
		ScicRegs.SCIHBAUD    =0x0001;  				// BaudRate=9600 baud @LSPCLK = 37.5MHz,BRR=0x1E7
		ScicRegs.SCILBAUD    =0x00E7;

		//ScicRegs.SCIHBAUD    =0x0000;  			// 19200 baud @LSPCLK = 37.5MHz.
		//ScicRegs.SCILBAUD    =0x00F3;

    	//ScicRegs.SCIHBAUD    =0x0000;  			// 115200 baud @LSPCLK = 37.5MHz.
        //ScicRegs.SCILBAUD    =0x0028;
	#endif
	#if (CPU_FRQ_100MHZ)							// 100 Mhz CPU Freq (20 MHz input freq)
		ScicRegs.SCIHBAUD    =0x0001;  				// 9600 baud @LSPCLK = 25MHz.
		ScicRegs.SCILBAUD    =0x0044;
	#endif

    ScicRegs.SCICTL1.bit.SWRESET = 1;				// Relinquish SCI from Reset
    ScicRegs.SCIFFTX.bit.SCIRST=1;

	ScicRegs.SCIFFRX.bit.RXFFIL  = SCI_FIFO_LEN;  	//����FIFO���
	ScicRegs.SCICTL1.bit.TXENA = 1;       			//ʹ�ܷ���
	ScicRegs.SCICTL1.bit.RXENA = 1;       			//ʹ�ܽ���
	// �ж����ò���-----1
	ScicRegs.SCIFFTX.bit.SCIFFENA = 1; 				//ʹ��FIFO�ж�
	ScicRegs.SCIFFRX.bit.RXFFIENA=1;
	EALLOW;
	// �ж����ò���-----2
	PieVectTable.SCIRXINTC = &uartRx_isr;
	EDIS;
	// �ж����ò���-----3
	PieCtrlRegs.PIEIER8.bit.INTx5 = 1;				//SCIC�����ж�
	// �ж����ò���-----4   			
	IER |= M_INT8;						  			

	ScicRegs.SCIFFCT.all=0x00;

	ScicRegs.SCIFFTX.bit.TXFIFOXRESET=1;
	ScicRegs.SCIFFRX.bit.RXFIFORESET=1;
}

//---------------------------------------------------------------------
//����һ���ֽ�,��ʱ����,SYSCLK = 60MHz
//---------------------------------------------------------------------
void scic_xmit(int a)
{
	Uint32 WaitTimer = 0;

	//while(ScicRegs.SCICTL2.bit.TXEMPTY != 1)
	while (ScicRegs.SCIFFTX.bit.TXFFST != 0)
	{
		WaitTimer++;
		if(WaitTimer > TIMEROUTSCI)break;
	}
	if(WaitTimer <= TIMEROUTSCI)
		ScicRegs.SCITXBUF=a;
}

//��������ֵ����16��������
void send_msg(char s[])
{
    int i;

	//TX_EN;
    for(i=0;i<16;i++)
    {
        scic_xmit(s[i]);
    }
	//Delay(3000);
	//RX_EN;
}

//�����ַ���ֵ����ascii������
void send_msgc(char *str,int len)
{
    int i;

	//TX_EN;
    for(i=0;i<len;i++)
    {
        scic_xmit(str[i]);
    }
	//Delay(3000);
	//RX_EN;
}

//---------------------------------------------------------------------
//sci_uart end
//---------------------------------------------------------------------
