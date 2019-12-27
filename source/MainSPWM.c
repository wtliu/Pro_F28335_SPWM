//==========================================================================================================
//                     F28335ϵͳ��������ع���---������˫����SPWM������������

//----------------------------------------------------------------------------------------------------------
// ���ߣ��׽� QQ:190459556
// ���ڣ�2018.12
// ���ܣ� 1��������˫����SPWM������������

// F28335Ƭ��RAM34KB��FLASH��256KB
//==========================================================================================================

/***********************************************************************************************************
 *                                          �������һ������ĿCCSV4.x
 * 1�������Ŀ��Ŀ¼�ṹ������Ҫ���ļ����Ƶ���ӦĿ¼
 * 
 * 2������include·��Include Options:
 * "${PROJECT_ROOT}/include"
 * 
 * 3��basic Options:
 * 0x200   --���F28027
 * 0x380   --���F28035
 * 0x380   --���F28335
***********************************************************************************************************/

#include "DSP28x_Project.h"
#include "Spwm.h"
#include "SysInit.h"
#include "LED.h"
#include "AT24C02.h"
#include "SCI_Uart.h"
#include <string.h>

//----------------------------------------------------------------------------------------------------------
//ADC����
#define ADC_CKPS   0x1   												// ADCģ��ʱ�� = HSPCLK/2*ADC_CKPS  = 30.0MHz/(1*2) = 15MHz; = 20.0MHz/(1*2) = 10MHz
#define ADC_SHCLK  0xf   												// S/H width in ADC module periods = 16 ADC clocks
//�ֱ��ǣ���������������ѹ��BUS��ѹ����������������������ѹ
Uint16 Id_REG[128], Ud_REG[128], Uf_REG[128], Uoa_REG[128],Uob_REG[128], Uoc_REG[128];
Uint16 Ioa_REG[128],Iob_REG[128],Ioc_REG[128],Vr1_REG[128],Ub_REG[128],A32_REG[128],A33_REG[128];
Uint16 Uw_REG[128],Uv_REG[128], Uu_REG[128];
//Uint32 Id,Ud,Uf,Uoa,Uob,Uoc;											//��Ӧ�������
float32 Id,Ud,Uf,Uoa,Uob,Uoc;
float32 Ioa,Iob,Ioc,Vr1,Ub,ADC;
//Uint32 Ioa,Iob,Ioc,Vr1,Ub,ADC;
Uint32 Sum,Max,Min;														//ADC�������ֵ���������ֵ��������Сֵ
Uint16 ConversionCount;													//ADC����������
Uint16 ADC_Flag,i,j;													//ADC������־,i,jѭ������
Uint16 MPPT_Flag;														//DCDC MPPT��־
void ADCInit(void);
interrupt void adc_isr(void);

//----------------------------------------------------------------------------------------------------------
//DCDC
Uint16 DCDC_N;															//DCDCռ�ձȵ��ڱ���
Uint16 DCDC_T;															//DCDC���ڱ���
Uint16 DCDC_LED;														//DCDCռ�ձ�����������
Uint16 DCDC_RED;														//DCDCռ�ձ�����������
Uint16 DC_PWM,pwm_step;													//MPPT���ռ�ձ�,pwm����ֵ
void InitSPwm4(void);
interrupt void epwm4_isr(void);
//DCDC PID
//float32 startpid,pmin,setmid,pmax,fastv;								//DCDC PID�����ֱ��ǣ�����PID��բ��ֵ����ѹ��Χ����Сֵ����ѹֵ����ѹ��Χ�����ֵ�����ٵ���ֵ
//Uint16 Is_UbPID = 0;													//�Ƿ���DCDC�ջ�PID 1-����
//Uint16 PWMD;															//PWM���ڷ���
//void Init_Ub_PID(void);
//void PWM_Ub_PID(void);

//----------------------------------------------------------------------------------------------------------
//DCAC
void InitEPwmTZ(void);
void InitSPwm1(void);
void InitSPwm2(void);
void InitSPwm3(void);
interrupt void epwm1_isr(void);
interrupt void epwm2_isr(void);
interrupt void epwm3_isr(void);
//DCAC PID
//����ʽPID����DCAC,PID���ڱ���,���Ա������г�ʼ��
float32 startpids,pmins,setmids,pmaxs,fastvs;							//SPWM1 PID�����ֱ��ǣ�����PID��բ��ֵ����ѹ��Χ����Сֵ����ѹֵ����ѹ��Χ�����ֵ�����ٵ���ֵ
Uint16 PWMDs;															//SPWM���ڷ���
Uint16 Is_UoPID = 0;													//�Ƿ���DCAC�ջ�PID 1-����
void Init_Uo_PID(void);
void PWM_Uo_PID(void);

//----------------------------------------------------------------------------------------------------------
//ECAP
Uint32 nCAP1,nCAP2,nCAP3;												//DSP����
float32 Freq,Phase;														//Ƶ��,��λ
Uint16 Freq_Lock=0;														//�Ƿ���Ƶ 1-����
Uint16 Phase_Lock=0;													//�Ƿ����� 1-����
Uint16 Freq_Flag1,Freq_Flag2,Freq_Flag3;								//Ƶ�ʱ�־
void Lock_Freq(void);
void InitECapture1(void);
void InitECapture2(void);
void InitECapture3(void);
interrupt void ecap1_isr(void);
interrupt void ecap2_isr(void);
interrupt void ecap3_isr(void);

//----------------------------------------------------------------------------------------------------------
//SCI
//#define COMM_ID		0xA2					//��������ʶ���ַ
#define COMM_ID			0xEE					//��������ʶ���ַ
#define ID_I			0						//���������0λ
#define CMD_I			1						//���������1λ
#define ADDR_I  		2						//���������2λ
#define DATA_I  		3						//���������3λ
#define DATB_I  		4						//���������4λ
Uint16 send_timer=0;							//LCD���ݷ���ʱ��
Uint16 timerNum=100;							//���ͼ������
Uart_Msg SCI_Msg={0, {0},0,0, 0};
unsigned char HandleCommAG(void);
void sendTFT(float32 U,Uint16 SID,Uint16 ID);
void getTFT(Uint16 SID,Uint16 ID);

//----------------------------------------------------------------------------------------------------------
//SYS
Uint16 DisType=0;								//��ʾ���ã�0-LCD12864 1-TFT 2-NoDisply
Uint16 DisTypeData=0;							//��ʾ�������ͣ�0-ԭʼ����ֵ  1-���Ա��ֵ

Uint16 IsProt=0;								//����״̬ 1-ʹ�ܱ���
Uint16 START_Flag=1;
void Delay(Uint16 i);
Uint16 KeyScan(void);
void itoa(int n,char s[]);
void float_TO_ascii(float a, char dat[8]);
void StopRun(void);

//----------------------------------------------------------------------------------------------------------
//SPI�������ʾ����
Uint16 Show_Flag,Show_Type,Show_Count;
Uint16 LedBuffer[4];													//�������ʾ��λ
Uint16 showdata;														//�������ʾ��ֵ
Uint16 sdata;  															//������ܷ��͵�����
void spi_xmit(Uint16 a);
void spi_fifo_init(void);
void spi_init(void);

//----------------------------------------------------------------------------------------------------------
//timer0
void timer0_init(void);
interrupt void cpu_timer0_isr(void);

//----------------------------------------------------------------------------------------------------------
//AT2408
Uint16 addr = 0;
Uint16 RecvBuf[16]={0};
Uint16 TranBuf[16]={0,1,2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF};
void ReadEEPROM(void);
void WriteEEPROM(void);

void main(void)
{
	//1����ʼ��ϵͳ
	InitSysCtrl();
	GPIOInit();									//��ʼ��IO��

	//2����ʼ��GPIO
	InitEPwm1Gpio();							//PWM1 IO��ʼ����DCACʹ��
	InitEPwm2Gpio();							//PWM2 IO��ʼ����DCACʹ��
	InitEPwm3Gpio();							//PWM3 IO��ʼ����DCACʹ��
	InitEPwm4Gpio();							//PWM4 IO��ʼ����DCDCʹ��
	InitECap1Gpio();							//Cap1 IO��ʼ����CAP1ʹ��
	InitECap2Gpio();							//Cap2 IO��ʼ����CAP2ʹ��
	InitECap3Gpio();							//Cap3 IO��ʼ����CAP3ʹ��
	InitSpiaGpio();  							//SPIA��ʼ���������ʹ��

	//3����������жϣ�������CPU�ж�
	DINT;

	//PIE��ʼ��
	InitPieCtrl();

	//����CPU�жϺ��������CPU�ж�
	IER = 0x0000;
	IFR = 0x0000;

	//PIE�������ʼ��
	InitPieVectTable();

///*
//-------------------------------------------------------------------------------------------------------- 
// ��дFLASH����ΪF28335.cmd�ļ����������2�д���,�����DSP2833x_MeMCopy.c,DSP280x_CSMPasswords.asm,F28335.cmd�ļ������±���
//--------------------------------------------------------------------------------------------------------  
MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	InitFlash();
//�������仰�����InitPieVectTable();���������һ�С� 
//-------------------------------------------------------------------------------------------------------- 
//*/

	//�����ж�
	EALLOW;

	PieVectTable.EPWM1_INT = &epwm1_isr;		//PWM1�ж�
	PieVectTable.EPWM2_INT = &epwm2_isr;		//PWM2�ж�
	PieVectTable.EPWM3_INT = &epwm3_isr;		//PWM3�ж�
	PieVectTable.EPWM4_INT = &epwm4_isr;		//PWM4�ж�
	PieVectTable.ECAP1_INT = &ecap1_isr;		//CAP1�ж�
	PieVectTable.ECAP2_INT = &ecap2_isr;		//CAP2�ж�
	PieVectTable.ECAP3_INT = &ecap3_isr;		//CAP3�ж�
	PieVectTable.ADCINT = &adc_isr;				//ADC�ж�

	EDIS;

	//4����ʼ�����е�����
	InitAdc();									//��ʼ��ADC

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;		//����ʱ�ӿ��ƼĴ���0
	EDIS;

	DCDC_N=95;									//=5%,��ռ�ձȷֳ�100�ݣ�ÿ��21��DCDC_NԽ��ռ�ձ�ԽС
	DCDC_T=2130;								//35.2KHz 30MHz=2130 T=28.4us
	DCDC_LED=60;								//��ʼ����=60ʱ3.4us
	DCDC_RED=60;								//��ʼ����
	DC_PWM=21*DCDC_N;
	InitSPwm4();								//PWM4���ܳ�ʼ��

	M=0.95;										//���Ʊ�
	N=400;										//�ز���400
	fs=50;										//50Hz
	Calc_Spwm();								//����PWM���ұ�

	SPWMCntA=0;  
	SPWMCntB=133;  
	SPWMCntC=266;  
	//����Ƶ��=F/(CarrVal/2)/1000 kHz
	PWMHz=(F/2000)/CarrVal;


	InitEPwmTZ();
	InitSPwm1();								//PWM1���ܳ�ʼ��
	InitSPwm2();								//PWM2���ܳ�ʼ��
	InitSPwm3();								//PWM3���ܳ�ʼ��
	InitECapture1();							//Cap1���ܳ�ʼ��
	InitECapture2();							//Cap2���ܳ�ʼ��
	InitECapture3();							//Cap3���ܳ�ʼ��

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;

	//5���û��ض��Ĵ��룬�����ж�

	PieCtrlRegs.PIEIER1.bit.INTx6 = 1;			//ʹ��ADC�ж�
	PieCtrlRegs.PIEIER3.bit.INTx1 = 1;			//ʹ��PWM1�ж�
	PieCtrlRegs.PIEIER3.bit.INTx2 = 1;			//ʹ��PWM2�ж�
	PieCtrlRegs.PIEIER3.bit.INTx3 = 1;			//ʹ��PWM3�ж�
	PieCtrlRegs.PIEIER3.bit.INTx4 = 1;			//ʹ��PWM4�ж�
	PieCtrlRegs.PIEIER4.bit.INTx1 = 1;			//ʹ��CAP1�ж�
	PieCtrlRegs.PIEIER4.bit.INTx2 = 1;			//ʹ��CAP2�ж�
	PieCtrlRegs.PIEIER4.bit.INTx3 = 1;			//ʹ��CAP3�ж�

	IER |= M_INT1+M_INT3+M_INT4;				//ʹ��CPU�ж�

	timer0_init();

	//6����ʼ���û�����
	Freq_Flag1=0;
	Freq_Flag2=0;
	Freq_Flag3=0;

	Freq=0;										//Ƶ��

	nCAP1=0;
	nCAP2=0;
	nCAP3=0;

	Id=0;Ud=0;Uf=0;Uoa=0;Uob=0;Uoc=0;
	Ioa=0;Iob=0;Ioc=0;Vr1=0;Ub=0;ADC=0;

	ADC_Flag=0;
	ConversionCount = 0;
	MPPT_Flag=1;
	ADCInit();									//ADC���ܳ�ʼ��

	timerNum=100;

	//BOOST PWM PID��ʼ��
	//Init_Ub_PID();
	//DCAC PWM PID��ʼ��
	Init_Uo_PID();

	Eerom_Gpio_Init();							// Initalize Eeprom 

	scic_init();  								// Initalize SCI for echoback

	Show_Flag=1;
	Show_Type=0;
	Show_Count=0;

	spi_fifo_init();							//��ʼ��Spi��FIFO
	spi_init();		  							//��ʼ��SPI
	//�������
	spi_xmit(0x0000);

	mRun_OFF();
	mErr_OFF();

	mPWM_OFF();									//��ֹPWM3 DCDC
	mSPWM_OFF();								//��ֹPWM1 SPWM
	//ȫ��ȫ���ж�
	EINT;										// ȫ��ȫ���ж� INTM
	ERTM;										// ȫ��ȫ��ʵʱ�ж� DBGM

	//7�� IDLE ѭ��
	for(;;)
	{
		asm(" NOP");

		if(ADC_Flag==1)
		{
			//---------------------------------ADC������ϣ���ʱ���Դ���----------------------------------
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Ioa_REG[i];
	
				if(Ioa_REG[i]<Min) 
					Min=Ioa_REG[i];
				else if(Ioa_REG[i]>Max) 
					Max=Ioa_REG[i];
			}
			Ioa=Max;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Iob_REG[i];
	
				if(Iob_REG[i]<Min) 
					Min=Iob_REG[i];
				else if(Iob_REG[i]>Max) 
					Max=Iob_REG[i];
			}
			Iob=Max;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Ioc_REG[i];
	
				if(Ioc_REG[i]<Min) 
					Min=Ioc_REG[i];
				else if(Ioc_REG[i]>Max) 
					Max=Ioc_REG[i];
			}
			Ioc=Max;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Vr1_REG[i];
	
				if(Vr1_REG[i]<Min) 
					Min=Vr1_REG[i];
				else if(Vr1_REG[i]>Max) 
					Max=Vr1_REG[i];
			}
			Vr1=(Sum-Max-Min)/126.0;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Uf_REG[i];
	
				if(Uf_REG[i]<Min) 
					Min=Uf_REG[i];
				else if(Uf_REG[i]>Max) 
					Max=Uf_REG[i];
			}
			Uf=(Sum-Max-Min)/126.0;						
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Ub_REG[i];
	
				if(Ub_REG[i]<Min) 
					Min=Ub_REG[i];
				else if(Ub_REG[i]>Max) 
					Max=Ub_REG[i];
			}
			Ub=(Sum-Max-Min)/126.0/4096.0*3.0;
			Ub=300.48*Ub;

			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Id_REG[i];
	
				if(Id_REG[i]<Min) 
					Min=Id_REG[i];
				else if(Id_REG[i]>Max) 
					Max=Id_REG[i];
			}
			Id=(Sum-Max-Min)/126.0/4096.0*3.0*500;								//Id�����������,ȥ�����ֵ����Сֵ
			Id=Id/97.85;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Ud_REG[i];
	
				if(Ud_REG[i]<Min) 
					Min=Ud_REG[i];
				else if(Ud_REG[i]>Max) 
					Max=Ud_REG[i];
			}
			Ud=(Sum-Max-Min)/126.0/4096.0;								//Ud�����ѹ����
			Ud=Ud*200.6*3.0;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Uoa_REG[i];
	
				if(Uoa_REG[i]<Min) 
					Min=Uoa_REG[i];
				else if(Uoa_REG[i]>Max) 
					Max=Uoa_REG[i];
			}
			Uoa=Max/4096.0*3;
			Uoa=(Uoa-1.5)*384.61;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Uob_REG[i];
	
				if(Uob_REG[i]<Min) 
					Min=Uob_REG[i];
				else if(Uob_REG[i]>Max) 
					Max=Uob_REG[i];
			}
			Uob=Max;
	
			Sum=0;Max=0;Min=0xffff;
			for(i=0;i<128;i++)
			{
				Sum+=Uoc_REG[i];
	
				if(Uoc_REG[i]<Min) 
					Min=Uoc_REG[i];
				else if(Uoc_REG[i]>Max) 
					Max=Uoc_REG[i];
			}
			Uoc=Max;
	
			ConversionCount = 0;
			ADC_Flag=0;
		}

		//===================================ADC�������ݴ��� end=======================================
		if(START_Flag==1)
		{
			START_Flag=0;

			//����PWM4 DCDC
			Delay(500);
			mPWM_ON();
			DCDC_N=95;

			//����PWM1 SPWM
			Delay(500);
			mSPWM_ON();

			IsProt=0;
			DisType=1;
			Is_UoPID=0;
			send_timer=0;
			timerNum=50;

			//��ϵͳ������ִ��һ�¸�λ����
			mOC_RESET_PRO_L();

			//��ȡ��������
			ReadEEPROM();
			Delay(500);
			if(RecvBuf[2]!=0xFF)
			{
				fs=RecvBuf[2];
				//Calc_Spwm();								//fs�����仯�����¼���SPWM
				//EPwm1Regs.TBPRD = CarrVal;				//�޸�fs���Ҳ����Ƶ��
				//EPwm2Regs.TBPRD = CarrVal;
				//EPwm3Regs.TBPRD = CarrVal;

				sendTFT(fs,0x02,0x07);
			}

			JDQ1_ON();
		}

		if(KEY1==1)
		{
			mSPWM_ON();
		}
		else
		{
			mSPWMA_ERR();
		}


		//PWM_Ub_PID();										//DCDC PID
		if(Is_UoPID==1) PWM_Uo_PID();						//ʹ��DCAC PIDʱ���������ϵļ̵���Ӧ�ô�������״̬�������ܽӵ�����ѹ
/*
		//KEY1�����ֶ�����ʹ��
		if(KeyScan()==1)
		{
			IsProt=2;										//���������ɹ�
			//����ʹ��
			mRun_ON();
			mOC_RESET_PRO_L();
			Delay(5000);
			mOC_RESET_PRO_H();
			mErr_OFF();
		}
*/
		if(IsProt==1)
		{
			IsProt=2;										//���������ɹ�
			//������λʹ��
			mRun_ON();
			mOC_RESET_PRO_L();
			Delay(5000);
			mOC_RESET_PRO_H();
			mErr_OFF();
		}

		if(IsProt==2)
		{
			//PWM1 IGBT����
			if(mERR==0)
			{
				mErr_ON();
				//TZ�������
			}
		}

/*		if(DisType==0)
		{
			if(send_timer>timerNum)
			{
				//ModbusЭ�鷢������
				//ע����������Ҫ���ó���16������ʾ
				//send_data[1]        = BusV()>>8;             	//BUS.V H8��8λ��
				//send_data[2]        = BusV()&0xFF;           	//BUS.V L8��8λ������ԭ������(��8λ<<8)|��8λ
				send_data[1]=Ud>>8;
				send_data[2]=Ud&0xFF;
				send_data[3]=Id>>8;
				send_data[4]=Id&0xFF;
				send_data[5]=Uoa>>8;
				send_data[6]=Uoa&0xFF;
				send_data[7]=Uob>>8;
				send_data[8]=Uob&0xFF;
				send_data[9]=Uoc>>8;
				send_data[10]=Uoc&0xFF;
				send_data[11]=Ioa>>8;
				send_data[12]=Ioa&0xFF;
				send_data[13]=0;
				send_data[14]=0;
				send_msg(send_data);
				send_timer=0;
			}
			else
			{
				send_timer++;
			}
		}


		*/
		if(DisType==1)
		{
			if(send_timer==timerNum)
			{
				sendTFT(Ud,0x01,0x01);
			}
			if(send_timer==timerNum*2)
			{
				sendTFT(Id,0x01,0x02);
			}
			if(send_timer==timerNum*3)
			{
				sendTFT(Ub,0x01,0x03);
			}
	
			if(send_timer==timerNum*4)
			{
				sendTFT(Uoa,0x01,0x04);
			}
			if(send_timer==timerNum*5)
			{
				sendTFT(Ioa,0x01,0x05);
			}
			if(send_timer==timerNum*6)
			{
				sendTFT(0.0,0x01,0x06);
			}
	
			if(send_timer==timerNum*7)
			{
				sendTFT(Uf,0x01,0x07);
			}
			if(send_timer==timerNum*8)
			{
				sendTFT(Vr1,0x01,0x08);
			}
			if(send_timer==timerNum*9)
			{
				sendTFT(PWMHz,0x01,0x09);
			}
	
			if(send_timer==timerNum*10)
			{
				sendTFT(Freq,0x01,0x0A);
				send_timer=0;
			}
			else
			{
				send_timer++;
			}
		}
/*
		//��ʾ����
		Show_Count++;										//���ڵ���SPI�������ʾ�������͵ı任ʱ��
		if(Show_Count==15)Show_Type++;						//�任ʱ����6s
		if(Show_Flag==1)
		{
		    if(Show_Type==1) showdata=Ud;
		    //if(Show_Type==2) showdata=Id;
		    //if(Show_Type==3) showdata=Uoa;
			//if(Show_Type==4) showdata=Uob;
			//if(Show_Type==5) showdata=Uoc;
		    //if(Show_Type==6) showdata=Io;

			//����Ҫ��ʾ����ֵ����A ==> ��������ʾ����==>��������λ����164==>8λ�������������==>��ʱ==>���ص�һ��ִ��
			//��ֵ������16λ����ֵ����
			CharDisplay(showdata,LedBuffer);
			//�ȷ����16λ��ֵ�����ڱ�����������λ����ܣ��������λ��һλ��ֵ
			//sdata=LedBuffer[2];
			//�ȷ����16λ��ֵ��������λ�����
			//sdata=(LedBuffer[3]<<8)+0x80;					//��С����
			sdata=(LedBuffer[3]<<8)+LedBuffer[2];
			spi_xmit(sdata);
			//�ٷŵ�16λ��ֵ
			sdata=(LedBuffer[1]<<8)+LedBuffer[0];
			spi_xmit(sdata);
			Delay(50000);
		}
		if(Show_Type>6)Show_Type=0;							//��ʾ6������
		if(Show_Count>60)Show_Count=1;
*/
		//�ȴ����ڵ������ַ��� ��������
		//SCI_Msg.timerOut�ڶ�ʱ��timer0 ��
 		if(SCI_Msg.timerOut >= RTU_TIMEROUT)
		{
			SCI_Msg.Mark_Para.Status_Bits.rFifoDataflag = 0;
			SCI_Msg.timerOut = 0;
			SCI_Msg.Mark_Para.Status_Bits.DISRevflag = 1;

			//��������
			HandleCommAG();

			SCI_Msg.Mark_Para.Status_Bits.DISRevflag = 0;
		}
	}
} 

void ReadEEPROM(void)
{
	//��Eeprom
	for(addr = 0;addr<=0xf;addr++)
	{
		RecvBuf[addr] = readbyte(addr);	
		delay(500);
	}
}

void WriteEEPROM(void)
{
	//AT24C08 дEEPROM����
	for(addr = 0;addr<=0xf;addr++)
	{
		writebyte(addr,TranBuf[addr]);
		delay(50000); 				// ���ʱ��һ��Ҫ��������ȻдEEPROM����ɹ�
	}
}

//---------------------------------------------------------------------
//������������Э�飬SCI�����������BaudRate=19200
//#define COMM_ID		0xA2		//��������ʶ���ַ
//#define ID_I			0			//���������0λ
//#define CMD_I			1			//���������1λ
//#define ADDR_I  		2			//���������2λ
//#define DATA_I  		3			//���������3λ
//#define DATB_I  		4			//���������4λ
//---------------------------------------------------------------------
unsigned char HandleCommAG()
{
	char str[10];										//char str[10];���岻�ܷŵ�main()ǰ��ȫ�ֶ����ϣ����򴮿�������������У�ԭ����
	char s1[]="Enter the command is not correct!\r\n";
	char s2[]="The command format is:A2 00 00\r\n";
	char s3[]="ReStart Power!\r\n";
	char s4[]="Setup OK!\r\n";
	char pa1[]="->Ud=";
	char pa2[]="->Id=";
	char pa3[]="->Ub=";
	char pa4[]="->Uoa=";
	char pa5[]="->Vr=";
	char pa6[]="->Ioa=";
	int len;
	unsigned char addr,getdata,getdatb,getdatc;
	//s1�ַ�����������ڵ��������ϣ������16���Ʋ鿴����ascii��
	if(SCI_Msg.rxData[ID_I] != COMM_ID)					//ID_I=0   COMM_ID=0xEE
	{
		SCI_Msg.rxWriteIndex = 0;
		SCI_Msg.rxReadIndex = 0;
		//��������ȷ��ֱ�ӷ���Ҳ���ڴ�д�����ȷҪִ�еĴ���
		len=strlen(s1);									//strlen #include <string.h>
		send_msgc(s1,len);
		return 1;
	}
	//��������
	switch(SCI_Msg.rxData[CMD_I])						//CMD_I=1��ʾ���ڽ��յ�������rxData�е�����ָ��λ��
	{
		//��ð���������ʾ����
		//EE  ����    ��ַ  ����
		//EE  00   00
		case 0: 
			addr =  SCI_Msg.rxData[ADDR_I];				//ADDR_I=2 ���������2λ
			//getdata = SCI_Msg.rxData[DATA_I];			//DATA_I=3 �õ�����ģʽֵ��������0x00-0xFF
			if(addr==0)
			{
				len=strlen(s2);
				send_msgc(s2,len);
			}
			break;
		//ϵͳ������������ϵͳ����
		//EE  01   00
		case 1: 
			addr =  SCI_Msg.rxData[ADDR_I];				//ADDR_I=2 ���������2λ
			if(addr==0)
			{
				//ϵͳ����
				START_Flag=1;
				len=strlen(s3);
				send_msgc(s3,len);
			}
			if(addr==1)
			{
				//LCD12864��ʾ
				DisType=0;
				len=strlen(s4);
				send_msgc(s4,len);
			}
			if(addr==2)
			{
				//TFTLCD��ʾ
				DisType=1;
				len=strlen(s4);
				send_msgc(s4,len);
			}
			if(addr==3)
			{
				//ȡ��������ʾ
				DisType=2;
				len=strlen(s4);
				send_msgc(s4,len);
			}
			if(addr==4)
			{
				//��ʾԭʼ����ֵ
				DisTypeData=0;
				len=strlen(s4);
				send_msgc(s4,len);
			}
			if(addr==5)
			{
				//��ʾ���Ա��ֵ
				DisTypeData=1;
				len=strlen(s4);
				send_msgc(s4,len);
			}
			break;
		//�ֶ�����ʹ�� 
		//EE  02   00
		case 2: 
			addr =  SCI_Msg.rxData[ADDR_I];				//ADDR_I=2 ���������2λ
			if(addr==0)
			{
				//����ʹ��
				IsProt=1;
			}
			if(addr==1)
			{
				//�������ص�Դʹ�ã�����ռ�ձ�
				DC_PWM=DC_PWM-75;
				//�޷�������ռ�ձ�
			   	if(DC_PWM<95) DC_PWM=95;
			   	if(DC_PWM>1995) DC_PWM=1995;
				sendTFT(DC_PWM,0x02,0x07);
			}
			if(addr==2)
			{
				//�������ص�Դʹ�ã���Сռ�ձ�
				DC_PWM=DC_PWM+75;
				//�޷�������ռ�ձ�
			   	if(DC_PWM<95) DC_PWM=95;
			   	if(DC_PWM>1995) DC_PWM=1995;
				sendTFT(DC_PWM,0x02,0x07);
			}
			if(addr==3)
			{
				//ֹͣϵͳ����
				StopRun();
			}
			if(addr==4)
			{
				//��ʼ����
				Phase_Lock=1;
			}
			len=strlen(s4);
			send_msgc(s4,len);
			break;
		//���TFT���ݲ���
		//ASCII(., 0, 1, 2, 3, 4, 5, 6, 7, 8,9)
		//=HEX(2E,30,31,32,33,34,35,36,37,38,39)
		//=10D(46,48,49,50,51,52,53,54,55,56,57)
		//�������TFT���ı��ؼ�07����Ϊ0 10 100ʱ
		//EE B1 11 00 02 00 07 11 [30 00] FF FC FF FF 
		//EE B1 11 00 02 00 07 11 [31 30 00] FF FC FF FF 
		//EE B1 11 00 02 00 07 11 [31 30 30 00] FF FC FF FF 
		case 0xB1: 
			addr =  SCI_Msg.rxData[6];
			if(addr==7)
			{
				if(SCI_Msg.rxData[9]==0)
				{
					//����rxData��16������-0x30���10������
					getdata = SCI_Msg.rxData[8]-0x30;
					TranBuf[2]=getdata;
				}
				else if(SCI_Msg.rxData[10]==0)
				{
					//�ϲ�[31 32Ϊһ��10������
					getdata = SCI_Msg.rxData[8]-0x30;
					getdatb = SCI_Msg.rxData[9]-0x30;
					TranBuf[2]=getdata*10+getdatb;
				}
				else if(SCI_Msg.rxData[11]==0)
				{
					getdata = SCI_Msg.rxData[8]-0x30;
					getdatb = SCI_Msg.rxData[9]-0x30;
					getdatc = SCI_Msg.rxData[10]-0x30;
					TranBuf[2]=getdata*100+getdatb*10+getdatc;
				}
		    	WriteEEPROM();
			}
			len=strlen(s4);
			send_msgc(s4,len);
			break;
		//�������ݲ���
		//EE  03   00
		case 3: 
			addr =  SCI_Msg.rxData[ADDR_I];				//ADDR_I=2 ���������2λ
			if(addr==0)
			{
				//EE B1 11 00 02 00 07 FF FC FF FF ;
				getTFT(0x2,0x7);
			}
			if(addr==1)
			{
				//��ȡ����
				ReadEEPROM();
				sendTFT(RecvBuf[2],0x02,0x07);
			}
			if(addr==2)
			{
				//�޸���λ
				phaseV=phaseV+1;
				//�޷�������ռ�ձ�
			   	if(phaseV<1) phaseV=1;
			   	if(phaseV>N) phaseV=N;
				sendTFT(phaseV,0x02,0x08);
			}
			if(addr==3)
			{
				//�޸���λ
				phaseV=phaseV-1;
				//�޷�������ռ�ձ�
			   	if(phaseV<1) phaseV=1;
			   	if(phaseV>N) phaseV=N;
				sendTFT(phaseV,0x02,0x08);
			}
			if(addr==4)
			{
				M=M+0.1;
				//�޷�������ռ�ձ�
			   	if(M<0.1) M=0.1;
			   	if(M>0.95) M=0.95;
				sendTFT(M,0x02,0x09);
			}
			if(addr==5)
			{
				M=M-0.1;
				//�޷�������ռ�ձ�
			   	if(M<0.1) M=0.1;
			   	if(M>0.95) M=0.95;
				sendTFT(M,0x02,0x09);
			}
			len=strlen(s4);
			send_msgc(s4,len);
			break;

		//����ʾ����EE 04 00

		//ModbusЭ�鷢��ASCII���ݵ�����
		//����ʾ����EE 05 00
		//ע����������Ҫ���óɲ���16������ʾ
		case 5:
			addr = SCI_Msg.rxData[ADDR_I];
			if(addr==0)
			{
				itoa((int)Ud, str);
				strcat(pa1,str);
				strcat(pa1,"\r\n");
				len=strlen(pa1);
				send_msgc(pa1,len);

				itoa((int)Id, str);
				strcat(pa2,str);
				strcat(pa2,"\r\n");
				len=strlen(pa2);
				send_msgc(pa2,len);

				itoa((int)Uoa, str);
				strcat(pa3,str);
				strcat(pa3,"\r\n");
				len=strlen(pa3);
				send_msgc(pa3,len);

				itoa((int)Uob, str);
				strcat(pa4,str);
				strcat(pa4,"\r\n");
				len=strlen(pa4);
				send_msgc(pa4,len);

				itoa((int)Uoc, str);
				strcat(pa5,str);
				strcat(pa5,"\r\n");
				len=strlen(pa5);
				send_msgc(pa5,len);

				itoa((int)Ioa, str);
				strcat(pa6,str);
				strcat(pa6,"\r\n");
				len=strlen(pa6);
				send_msgc(pa6,len);
			}
			break;

		default:break;
	}

	SCI_Msg.rxWriteIndex = 0;
	SCI_Msg.rxReadIndex = 0;

	return 0;
}

//---------------------------------------------------------------------
//��ʱ��0��ʼ��1ms �ж�����
//---------------------------------------------------------------------
void timer0_init()
{
	//CpuTimer0��ʼ��
    InitCpuTimers();

	//�ж����ò���-----1,����ģ���ж�ʹ�ܣ�λ�� Timer->RegsAddr->TCR.bit.TIE = 1;
	//ConfigCpuTimer(&CpuTimer0, 150, 1000);		//��ʱʱ��1000=1ms
	ConfigCpuTimer(&CpuTimer0, 150, 500);			//��ʱʱ��500=0.5ms
    CpuTimer0Regs.TCR.all = 0x4001;             	//Use write-only instruction to set TSS bit = 0

	//�ж����ò���-----2����ӳ���жϷ�����
	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;
	//�ж����ò���-----3������CPU�ж�Y
	IER |= M_INT1;
	//�ж����ò���-----4������Y�ж���ĵڼ�λ
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount++;

	//AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;						//timer0�����������ADC SEQ1����ʱʱ��1000=1ms AD����Ҳ��1ms

	//����ͨѶ����
	if(SCI_Msg.Mark_Para.Status_Bits.rFifoDataflag == 1)
	{
	   SCI_Msg.timerOut++;
	}

	if(CpuTimer0.InterruptCount>1000)
	{
		CpuTimer0.InterruptCount=0;
	}
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

Uint16 KeyScan(void)
{
	static Uint16 key_up=1;									//�������ɿ���־
	if(key_up&&(KEY1==0))
	//if(key_up&&(KEY1==0||KEY2==0||KEY3==0))					//�м�����
	{
		Delay(10000);										//ȥ����
		key_up=0;											//��ʾ����û�ɿ�
		if(KEY1==0)											//����KEY1
		{
  			return 1;
		}
	}
	else if(KEY1==1)
	{
		key_up=1;
	}
	return 0;

/*		else if(KEY2==0)
		{
			return 2;
		}
		else if(KEY3==0)
		{
			return 3;
		}
	}
	else if(KEY1==1||KEY2==1||KEY3==1)
	{
		key_up=1;
	}
	return 0;			*/									//�ް�������
}

//������n�������ת��Ϊ��Ӧ���ַ��������浽s��
void itoa(int n,char s[])  
{
     int i = 0;  
     int left = 0;  
     int right = 0;  
     while( n )       			//������ת��Ϊ�ַ�  
     {  
        s[i] = n % 10 + '0';  
        n /= 10;  
        i++;         			//left�Ѿ������λ  
     }  
     right = i - 1;  
     s[i] = '\0';        		//����ַ���������־  
     while(left < right)    	//�������е�Ԫ������  
     {  
         char tmp = s[left];  
         s[left] = s[right];  
         s[right] = tmp;  
         left++;  
         right--;  
      }  
}  

//������������ΪASCII��
//6λ��Ч���֣�����Ϊ������ �磺0.01234����6λ��Ч����
void float_TO_ascii(float a, char dat[8])
{
        if(1000<=a&&a<10000)
        {
                dat[0] = (int)a%10000/1000 + 0x30;
                dat[1] = (int)a%1000/100 + 0x30;
                dat[2] = (int)a%100/10 + 0x30;
                dat[3] = (int)a%10 + 0x30;
                dat[4] = 0x2e;
                dat[5] = (int)(a*10)%10 + 0x30;
                dat[6] = (int)(a*100)%10 + 0x30;
                dat[7] = 0;
        }
        if(100<=a&&a<1000)
        {
                dat[0] = (int)a%1000/100 + 0x30;
                dat[1] = (int)a%100/10 + 0x30;
                dat[2] = (int)a%10 + 0x30;
                dat[3] = 0x2e;
                dat[4] = (int)(a*10)%10 + 0x30;
                dat[5] = (int)(a*100)%10 + 0x30;
                dat[6] = (int)(a*1000)%10 + 0x30;
                dat[7] = (int)(a*10000)%10 + 0x30;
        }
        if(10<=a&&a<100)
        {
                dat[0] = (int)a%100/10 + 0x30;
                dat[1] = (int)a%10 + 0x30;
                dat[2] = 0x2e;
                dat[3] = (int)(a*10)%10 + 0x30;
                dat[4] = (int)(a*100)%10 + 0x30;
                dat[5] = (int)(a*1000)%10 + 0x30;
                dat[6] = (int)(a*10000)%10 + 0x30;
                dat[7] = (int)(a*100000)%10 + 0x30;
        }
        if(1<=a&&a<10)
        {
                dat[0] = (int)a%10 + 0x30;
                dat[1] = 0x2e;
                dat[2] = (int)(a*10)%10 + 0x30;
                dat[3] = (int)(a*100)%10 + 0x30;
                dat[4] = (int)(a*1000)%10 + 0x30;
                dat[5] = (int)(a*10000)%10 + 0x30;
                dat[6] = (int)(a*100000)%10 + 0x30;
                dat[7] = 0;
        }
        if(0<=a&&a<1)
        {
                dat[0] = 0x30;
                dat[1] = 0x2e;
                dat[2] = (int)(a*10)%10 + 0x30;
                dat[3] = (int)(a*100)%10 + 0x30;
                dat[4] = (int)(a*1000)%10 + 0x30;
                dat[5] = (int)(a*10000)%10 + 0x30;
                dat[6] = (int)(a*100000)%10 + 0x30;
                dat[7] = 0;
        }
        if(-1<a&&a<0)
        {
                dat[0] = 0x2d;
                dat[1] = 0x30;
                dat[2] = 0x2e ;
                dat[3] = (int)(-a*10)%10 + 0x30;
                dat[4] = (int)(-a*100)%10 + 0x30;
                dat[5] = (int)(-a*1000)%10 + 0x30;
                dat[6] = (int)(-a*10000)%10 + 0x30;
                dat[7] = (int)(-a*100000)%10 + 0x30;
        }
        if(-10<a&&a<=-1)
        {
                dat[0] = 0x2d;
                dat[1] = (int)(-a)%10 + 0x30;
                dat[2] = 0x2e ;
                dat[3] = (int)(-a*10)%10 + 0x30;
                dat[4] = (int)(-a*100)%10 + 0x30;
                dat[5] = (int)(-a*1000)%10 + 0x30;
                dat[6] = (int)(-a*10000)%10 + 0x30;
                dat[7] = (int)(-a*100000)%10 + 0x30;
        }
        if(-100<a&&a<=-10)
        {
                dat[0] = 0x2d;
                dat[1] = (int)(-a)%100/10 + 0x30;
                dat[2] = (int)(-a)%10 + 0x30;
                dat[3] = 0x2e ;
                dat[4] = (int)(-a*10)%10 + 0x30;
                dat[5] = (int)(-a*100)%10 + 0x30;
                dat[6] = (int)(-a*1000)%10 + 0x30;
                dat[7] = (int)(-a*10000)%10 + 0x30;
        }
        if(-1000<a&&a<=-100)
        {
                dat[0] = 0x2d;
                dat[1] = (int)(-a)%1000/100 + 0x30;
                dat[2] = (int)(-a)%100/10 + 0x30;
                dat[3] = (int)(-a)%10 + 0x30;
                dat[4] = 0x2e ;
                dat[5] = (int)(-a*10)%10 + 0x30;
                dat[6] = (int)(-a*100)%10 + 0x30;
                dat[7] = (int)(-a*1000)%10 + 0x30;
        }
        if(-10000<a&&a<=-1000)
        {
                dat[0] = 0x2d;
                dat[1] = (int)(-a)%10000/1000 + 0x30;
                dat[2] = (int)(-a)%1000/100 + 0x30;
                dat[3] = (int)(-a)%100/10 + 0x30;
                dat[4] = (int)(-a)%10 + 0x30;
                dat[5] = 0x2e ;
                dat[6] = (int)(-a*10)%10 + 0x30;
                dat[7] = (int)(-a*100)%10 + 0x30;
        }
}

void StopRun()
{
	//ֹͣPWM4 DCDC
	DCDC_N=95;
	mPWM_OFF();

	//ֹͣPWM1 SPWM
	Delay(500);
	mSPWM_OFF();

	IsProt=0;
	DisType=0;

	mRun_OFF();
	JDQ1_OFF();
}

void sendTFT(float32 U,Uint16 SID,Uint16 ID)
{
	//���ݴ���豸����ǰ��Ĭ�� RS232 ��ƽ�����û���Ҫ TTL ͨѶ��ʽ��ֻ�轫 J5 ���̶�·
	//��·��Ͳ�����RS232���д��ڲ��ԣ���������VisualTFT���ع���
	//�ڹ��������п���������F28035�Ĵ��䲨���� 
	//TTL ��ƽģʽ�¼��� 3.3V �� 5V IO �������
	//SCI�������ݣ�������19200
	//ASCII(., 0, 1, 2, 3, 4, 5, 6, 7,  8, 9)
	//=HEX(2E,30,31,32, 33,34,35,36,37,38,39)
	//EE B1 10 Screen_id Control_id  	Strings    		FF FC FF FF
	//EE B1 10   00 01     00 01     32 33 2E 30 35     FF FC FF FF =23.05
	// 0  1  2    3  4     5   6     7   8  9  10  11   12 13 14 15
	char str[10];										//char str[10];���岻�ܷŵ�main()ǰ��ȫ�ֶ����ϣ����򴮿�������������У�ԭ����
	//ftoa()ʹ��ʱϵͳ�����������У������ǣ����1��2·���ؿ��������ڷ���A2 03 00����ʱ��1·���ص�PWM������ֹͣ
	//�����ʹ��ftoa()ϵͳ���������У�Ҳ���������������Ӧ����һ�����õĺ���
	float_TO_ascii(U,str);

	send_Tdata[4] = SID;					//ҳ��ؼ�ID

	send_Tdata[6] = ID;						//�ؼ�ID

	send_Tdata[7] = str[0];					//send_Tdata[7-11]ΪASCII��ֵ
	send_Tdata[8] = str[1];
	send_Tdata[9] = str[2];
	send_Tdata[10]= str[3];
	send_Tdata[11]= str[4];

	send_msg(send_Tdata);					//��������
}

//�õ�TFT�Ŀؼ�����
void getTFT(Uint16 SID,Uint16 ID)
{
	send_Rdata[4] = SID;					//ҳ��ؼ�ID
	send_Rdata[6] = ID;						//�ؼ�ID
	send_msg(send_Rdata);					//��������
}

/*
//---------------------------------------------------------------------
//DCDC PID��ʼ��
//---------------------------------------------------------------------
void Init_Ub_PID(void)
{
	//����160VDC���400VDC��PID����
    startpid=0.8;
    pmin=2.17;						//setmid-pmin=0.04<fastv
    setmid=2.21;
    pmax=2.25;						//pmax-setmid=0.04<fastv
    fastv=0.05;	
    PWMD=10;						//PWM���ڷ���
}

//---------------------------------------------------------------------
//DCDC PID
//---------------------------------------------------------------------
void PWM_Ub_PID(void)
{
	//----------------------------PID----------------------------------
	//���ƫ�󣬼�Сռ�ձ�
	if(Ub>pmax)
	{
		DC_PWM+=1;					//ϸ��PWM
		if((Ub-setmid)>fastv)
		{
			DC_PWM+=PWMD;			//���ٵ���PWM����
		}
	}
	//����ȶ�������ռ�ձ�
	if((Ub<=pmax)&&(Ub>=pmin)){DC_PWM+=0;}
	//�����С������ռ�ձ�
	if((Ub<pmin)&&(Ub>startpid))
	{
		DC_PWM-=1;					//ϸ��PWM
		if((setmid-Ub)>fastv)
		{
			DC_PWM-=PWMD;			//���ٵ���PWM����
		}
	}
	//�������ѹ������ռ�ձ���С�������С
	if(Ub<startpid){DC_PWM=1995;}
	//�޷�������ռ�ձ�
   	if(DC_PWM<1155) DC_PWM=1155;
   	if(DC_PWM>1995) DC_PWM=1995;
}
*/
//---------------------------------------------------------------------
//DCAC PID��ʼ��
//---------------------------------------------------------------------
void Init_Uo_PID(void)
{
	//����400VDC���225VAC��PID����
    startpids=1.5;
    pmins=2.97;								//220VAC setmid-pmin=0.04<fastv
    setmids=3.01;							//230VAC
    pmaxs=3.05;								//235VAC pmax-setmid=0.04<fastv
    fastvs=0.05;	
    PWMDs=0.05;
}

//---------------------------------------------------------------------
//DCAC����ʽPID�㷨
//---------------------------------------------------------------------
void PWM_Uo_PID(void)
{
	//----------------------------PID----------------------------------
	///*
	//�������ѹ������ռ�ձ���С�������С
	if(Ud>startpids)
	{
		//���ƫ�󣬼�Сռ�ձ�
		if(Uoa>pmaxs)
		{
			M-=0.01;					//ϸ��SPWM
			if((Uoa-setmids)>fastvs)
			{
				M-=PWMDs;				//���ٵ���SPWM����
			}
		}
		//����ȶ�������ռ�ձ�
		if((Uoa<=pmaxs)&&(Uoa>=pmins)){M+=0;}
		//�����С������ռ�ձ�
		if((Uoa<pmins)&&(Uoa>startpids))
		{
			M+=0.01;					//ϸ��SPWM
			if((setmids-Uoa)>fastvs)
			{
				M+=PWMDs;				//���ٵ���SPWM����
			}
		}
		//�޷�������ռ�ձ�
	   	if(M<0.1) M=0.1;
	   	if(M>0.95) M=0.95;
   		Calc_Spwm();
	}
}

//ADC�ж�
interrupt void  adc_isr(void)
{
	Ub_REG[ConversionCount]  = AdcRegs.ADCRESULT0 >>4;
	Id_REG[ConversionCount]  = AdcRegs.ADCRESULT1 >>4;
	Uu_REG[ConversionCount]  = AdcRegs.ADCRESULT2 >>4;
	Vr1_REG[ConversionCount] = AdcRegs.ADCRESULT3 >>4;

	Uoa_REG[ConversionCount] = AdcRegs.ADCRESULT4 >>4;
	Iob_REG[ConversionCount] = AdcRegs.ADCRESULT5 >>4;
	Ioc_REG[ConversionCount] = AdcRegs.ADCRESULT6 >>4;
	Ud_REG[ConversionCount]  = AdcRegs.ADCRESULT7 >>4;

	Ioa_REG[ConversionCount] = AdcRegs.ADCRESULT8 >>4;
	Uob_REG[ConversionCount] = AdcRegs.ADCRESULT9 >>4;
	Uoc_REG[ConversionCount] = AdcRegs.ADCRESULT10>>4;
	Uf_REG[ConversionCount]  = AdcRegs.ADCRESULT11>>4;

	Uv_REG[ConversionCount]  = AdcRegs.ADCRESULT12>>4;//...
	A32_REG[ConversionCount] = AdcRegs.ADCRESULT13>>4;
	A33_REG[ConversionCount] = AdcRegs.ADCRESULT14>>4;
	Uw_REG[ConversionCount]  = AdcRegs.ADCRESULT15>>4;
	
	//���128������������˾ͽ�����һ��ADC����
	if(ConversionCount == 127)
	{
		ADC_Flag=1;
	 	ConversionCount = 0;
	}
	else
	{
		ConversionCount++;
	}

	//���³�ʼ��Ϊ��ADC����
	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1; 			// ���� SEQ1
	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1; 		// ���INT SEQ1λ
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; 	// ȷ���жϵ�PIE

	return;
}

void Lock_Freq(void)
{
	//------------------------------------����Ƶ�� begin------------------------------------------ 
	//��ϵͳ����DSP�Ĳ���ģ��ͨ���Ե����ο����㷽���ź������صĲ�������ȡƵ��,ͨ��DSP�ڲ��ļ������õ����㷽���ź�ǰ�����������ص�ʱ��
	//��Ϊ���㷽���źŵ����ڣ��Ӷ��õ����Ҳ���Ƶ�ʣ������㷽���źŵ�ÿ�������ض���Ӧ�ŵ����ο���ѹ���Ҳ��Ĺ���㣬
	//-------------------------------------------------------------------------------------------
	if(Freq_Flag1)
	{
		Freq=150000000/nCAP1;

		//Ƶ�ʵ�����Ƶ����44-56Hz�ڣ�����Ƶ��
		//Ƶ�ʸ��ٲ��Է������ȿ�����䣬Ȼ��ͨ��k1,k2��������Ƶ�ʣ���������53Hz(44-56Hz)
		//Ȼ���ͨ��������ʱ������Ҳ�Ƶ�����Ͼͻ������Ƶ��һ�£���ʾ���Գɹ���ע�˹�����ǧ��Ҫ�ϲ����̵�������Ϊû������������������
		//��������½���Ƶ�ʸ���
		if(Is_UoPID==0)
		{
			if((Freq>=44)&&(Freq<=56))
			{
				if(((fs-Freq)>0.3)||((Freq-fs)>0.3))			//fs��ʼֵ��50Hz��׼Ƶ��
				{
					fs=Freq;
					Calc_Spwm();								//fs�����仯�����¼���SPWM
					EPwm1Regs.TBPRD = CarrVal;					//�޸�fs���Ҳ����Ƶ��
					EPwm2Regs.TBPRD = CarrVal;					//�޸�fs���Ҳ����Ƶ��
					EPwm3Regs.TBPRD = CarrVal;					//�޸�fs���Ҳ����Ƶ��
				}
			}
			Freq_Flag1 = 0;
			Freq_Lock = 1;										//��Ƶ���
		}
	}
	//-----------------------------Ƶ�ʺ���λ������� end-------------------------------------------- 
}

//SPWM �ж�
//ePWMģ���ܹ��ڱ�֤ϵͳ������С��ǰ���¿��ṩ0%~100%ռ�ձȣ������ֹ���ģʽ���ӷ�����ģʽ���������ģʽ�ͼ�������ģʽ��
//��ϵͳ���ÿ��棨Up-Down������ģʽ��PWM���ζԳ�)�����ӷ�����ֵ�ﵽ��CMPAֵƥ�䣬��λePWM1A�����
//����������ֵ�ﵽ��CMPAֵƥ��,ePWM1A�����λ�����CMPAֵ���������ֵ��ƥ�䣬�����ISR��������Ӱ�Ĵ�����

//����DSP��ePWM���ɿ���Ƶ�ʼ����ePWM�����ڣ���ÿһ��ePWM�жϵ���ʱ���ɹ�����������㱾����PWM����ռ�ձȣ����ɵõ���Ӧ��SPWM����
//��Ƶ�ʷ����ı䣬��ֻ��ı�ePWM�����ڡ�����Ҫ����������ε���λ�����ƶ����Ҳ����ָ�뼴�ɡ�
interrupt void epwm1_isr(void)
{
	SPWMCntA++;

	if(SPWMCntA>=N)
	{
		SPWMCntA=0;
	}

	SinCntA=(Uint16)(SPWMCntA*128/N);
	EPwm1Regs.CMPA.half.CMPA = CompVal[SinCntA];		//CMPA �������Ƚ�A �Ĵ�����,CompVal[]��calc_spwm()������,SinCnt=0-128

	EPwm1Regs.ETCLR.bit.INT=1;
	PieCtrlRegs.PIEACK.all =PIEACK_GROUP3;
}

interrupt void epwm2_isr(void)
{
	SPWMCntB++;

	if(SPWMCntB>=N)
	{
		SPWMCntB=0;
	}

	SinCntB=(Uint16)(SPWMCntB*128/N);
	EPwm2Regs.CMPA.half.CMPA = CompVal[SinCntB];		//CMPA �������Ƚ�A �Ĵ�����,CompVal[]��calc_spwm()������,SinCnt=0-128

   	EPwm2Regs.ETCLR.bit.INT=1;
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm3_isr(void)
{
	SPWMCntC++;

	if(SPWMCntC>=N)
	{
		SPWMCntC=0;
	}

	SinCntC=(Uint16)(SPWMCntC*128/N);
	EPwm3Regs.CMPA.half.CMPA = CompVal[SinCntC];		//CMPA �������Ƚ�A �Ĵ�����,CompVal[]��calc_spwm()������,SinCnt=0-128

	EPwm3Regs.ETCLR.bit.INT=1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//DCDC��PWM
interrupt void epwm4_isr(void)
{
	//����PWM��Ƶ�ʣ�ռ�ձ�CMPAֵ��TBPRD���ڵĹ�ϵ��
	//1��TBPRD����һ����PWMƵ��һ��
	//2��TBPRDֵԽ��PWMƵ��ԽС
	//3��PWMƵ��ԽС��ռ�ձ�CMPAֵ����ȡ��Խ�󣬷�֮
	//����TBPRDֵ��CMPAֵȡ�����н�����
	EPwm4Regs.CMPA.half.CMPA=DC_PWM;

	EPwm4Regs.DBRED = DCDC_LED;
	EPwm4Regs.DBFED = DCDC_RED;

	EPwm4Regs.ETCLR.bit.INT = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//�����ж�
interrupt void ecap1_isr(void)
{
	nCAP1=ECap1Regs.CAP1;					//��׽����

	Freq_Flag1=1;
	Lock_Freq();
	if(Phase_Lock==1)
	{
		if(Freq_Lock) 
		{
			//SPWMCntA�ǵ������������λ��ģ���λ��Խ�󣬲����������Խ��������բ���ջٹ��ʹ�
			SPWMCntA=phaseV;
			Freq_Lock = 0;						//����һ�´���Ƶ
		}
	}

	ECap1Regs.ECCLR.bit.CEVT1 = 1;			//ECCLR ��׽�ж�����Ĵ���,CEVT1��DSP2833x_ECap.h�ļ��ж���
	ECap1Regs.ECCLR.bit.INT = 1;
	ECap1Regs.ECCTL2.bit.REARM = 1;			//ECCTL2 ��׽���ƼĴ���2

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

interrupt void ecap2_isr(void)
{
	nCAP2=ECap1Regs.TSCTR;					//TSCTR ʱ���������
	Freq_Flag2=1;

	if(Phase_Lock==1)
	{
		SPWMCntB=phaseV;
	}

	ECap2Regs.ECCLR.bit.CEVT1 = 1;
	ECap2Regs.ECCLR.bit.INT = 1;
	ECap2Regs.ECCTL2.bit.REARM = 1;

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

interrupt void ecap3_isr(void)
{
	nCAP3=ECap1Regs.TSCTR;					//TSCTR ʱ���������
	Freq_Flag3=1;

	if(Phase_Lock==1)
	{
		SPWMCntC=phaseV;
	}

	ECap3Regs.ECCLR.bit.CEVT1 = 1;
	ECap3Regs.ECCLR.bit.INT = 1;
	ECap3Regs.ECCTL2.bit.REARM = 1;

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

//��ʼ������ģ��Uref
void InitECapture1()
{
	ECap1Regs.ECEINT.all = 0x0000; 			// �������в�׽�ж�
	ECap1Regs.ECCLR.all = 0xFFFF;  			// �������CAP�жϱ�־
	ECap1Regs.ECCTL1.bit.CAPLDEN = 0; 		// ��ֹCAP1-CAP4�Ĵ�������
	ECap1Regs.ECCTL2.bit.TSCTRSTOP = 0;  	// ȷ��������ֹͣ

	//��������Ĵ���
	ECap1Regs.ECCTL2.bit.CONT_ONESHT = 1;	// ����
	ECap1Regs.ECCTL2.bit.STOP_WRAP = 0;  	// ͣ��4�¼�
	ECap1Regs.ECCTL1.bit.CAP1POL = 0; 		// ������
	ECap1Regs.ECCTL1.bit.CTRRST1 = 1; 		// �������
	ECap1Regs.ECCTL2.bit.SYNCI_EN = 1;		// ������ͬ��
	ECap1Regs.ECCTL2.bit.SYNCO_SEL = 0;  	// ͨ��
	ECap1Regs.ECCTL1.bit.CAPLDEN = 1; 		// ���ò���Ԫ

	ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;  	// ����������
	ECap1Regs.ECCTL2.bit.REARM= 1;			// arm one-shot
	ECap1Regs.ECEINT.bit.CEVT1 = 1;			// 4 events = interrupt
}

void InitECapture2()
{
	ECap2Regs.ECEINT.all = 0x0000; 			// Disable all capture interrupts
	ECap2Regs.ECCLR.all = 0xFFFF;  			// Clear all CAP interrupt flags
	ECap2Regs.ECCTL1.bit.CAPLDEN = 0; 		// Disable CAP1-CAP4 register loads
	ECap2Regs.ECCTL2.bit.TSCTRSTOP = 0;  	// Make sure the counter is stopped

	// Configure peripheral registers
	ECap2Regs.ECCTL2.bit.CONT_ONESHT = 1;	// One-shot
	ECap2Regs.ECCTL2.bit.STOP_WRAP = 0;  	// Stop at 4 events
	ECap2Regs.ECCTL1.bit.CAP1POL = 0; 		// Rising edge
	ECap2Regs.ECCTL1.bit.CTRRST1 = 1; 		// Difference operation
	ECap2Regs.ECCTL2.bit.SYNCI_EN = 1;		// Enable sync in
	ECap2Regs.ECCTL2.bit.SYNCO_SEL = 0;  	// Pass through
	ECap2Regs.ECCTL1.bit.CAPLDEN = 1; 		// Enable capture units

	ECap2Regs.ECCTL2.bit.TSCTRSTOP = 1;  	// Start Counter
	ECap2Regs.ECCTL2.bit.REARM= 1;			// arm one-shot
	ECap2Regs.ECEINT.bit.CEVT1 = 1;			// 4 events = interrupt
}

void InitECapture3()
{
	ECap3Regs.ECEINT.all = 0x0000; 			// Disable all capture interrupts
	ECap3Regs.ECCLR.all = 0xFFFF;  			// Clear all CAP interrupt flags
	ECap3Regs.ECCTL1.bit.CAPLDEN = 0; 		// Disable CAP1-CAP4 register loads
	ECap3Regs.ECCTL2.bit.TSCTRSTOP = 0;  	// Make sure the counter is stopped

	// Configure peripheral registers
	ECap3Regs.ECCTL2.bit.CONT_ONESHT = 1;	// One-shot
	ECap3Regs.ECCTL2.bit.STOP_WRAP = 0;  	// Stop at 4 events
	ECap3Regs.ECCTL1.bit.CAP1POL = 0; 		// Rising edge
	ECap3Regs.ECCTL1.bit.CTRRST1 = 1; 		// Difference operation
	ECap3Regs.ECCTL2.bit.SYNCI_EN = 1;		// Enable sync in
	ECap3Regs.ECCTL2.bit.SYNCO_SEL = 0;  	// Pass through
	ECap3Regs.ECCTL1.bit.CAPLDEN = 1; 		// Enable capture units

	ECap3Regs.ECCTL2.bit.TSCTRSTOP = 1;  	// Start Counter
	ECap3Regs.ECCTL2.bit.REARM= 1;			// arm one-shot
	ECap3Regs.ECEINT.bit.CEVT1 = 1;			// 4 events = interrupt
}


void InitEPwmTZ()
{
   // Enable TZ1 and TZ2 as one shot trip sources
   EALLOW;

   // What do we want the TZ1 and TZ2 to do?
//  EPwm1Regs.TZCTL.bit.TZA = 0x3;
//  EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
   EPwm2Regs.TZCTL.bit.TZA = 0x3;
   EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
  EPwm3Regs.TZCTL.bit.TZA = TZ_FORCE_LO;
   EPwm3Regs.TZCTL.bit.TZB = 0x3;

   EDIS;
}


//��ʼ����ePWMģ��SPWM
//ePWMģ�������¼�����ģ�鹹�ɣ�ʱ��(TB)��ģ�顢������-�Ƚ���(CC)��ģ�顢�����޶�(AQ)��ģ�顢����(DB)��������ģ�顢PWMն����(PC)��ģ�顢
//���϶�·��(Trip Zone)��ģ�顢�¼�������(ET)��ģ�顣����ePWMģ��ʱ��Ҫ��������ģ���еļĴ������г�ʼ����
void InitSPwm1()
{
	EPwm1Regs.TBPRD = CarrVal;						// Set timer period
	EPwm1Regs.TBPHS.half.TBPHS = 0x0000;            // Phase is 0
	EPwm1Regs.TBCTR = 0x0000;                       // Clear counter

	EPwm1Regs.CMPA.half.CMPA = 0;					//ϵͳ�ϵ�ʱռ�ձ�Ϊ0

	// Setup TBCLK
	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 	// Count up
	EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;        	// Disable phase loading
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       	// Clock ratio to SYSCLKOUT
	EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;          	// Slow so we can observe on the scope

	// Setup shadowing CMPCTL�������ȽϿ��ƼĴ���
	EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  	// Load on Zero
	EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

	// Set actions
	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;              // Set PWM1A on CAU
	EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Clear PWM1A on CAD

	EPwm1Regs.AQCTLB.bit.CAU = AQ_CLEAR;            // Clear PWM3B on CAU
	EPwm1Regs.AQCTLB.bit.CAD = AQ_SET;              // Set PWM1B on CAD

	EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
	EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;

	EPwm1Regs.DBRED = 145;							//1.5us����
	EPwm1Regs.DBFED = 145;
   
	EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       // Select INT on Zero event
	EPwm1Regs.ETSEL.bit.INTEN = 1;                  // Enable INT
	EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;             // Generate INT on 1st event   
}

void InitSPwm2()
{
	EPwm2Regs.TBPRD = CarrVal;						// Set timer period
	EPwm2Regs.TBPHS.half.TBPHS = 0x0000;            // Phase is 0
	EPwm2Regs.TBCTR = 0x0000;                       // Clear counter

	EPwm2Regs.CMPA.half.CMPA = 0;					//ϵͳ�ϵ�ʱռ�ձ�Ϊ0

	// Setup TBCLK
	EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 	// Count up
	EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;        	// Disable phase loading
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       	// Clock ratio to SYSCLKOUT
	EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;          	// Slow so we can observe on the scope

	// Setup shadowing CMPCTL�������ȽϿ��ƼĴ���
	EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  	// Load on Zero
	EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

	// Set actions
	EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;              // Set PWM1A on CAU
	EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Clear PWM1A on CAD

	EPwm2Regs.AQCTLB.bit.CAU = AQ_CLEAR;            // Clear PWM3B on CAU
	EPwm2Regs.AQCTLB.bit.CAD = AQ_SET;              // Set PWM1B on CAD

	EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
	EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;

	EPwm2Regs.DBRED = 145;
	EPwm2Regs.DBFED = 145;

	EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       // Select INT on Zero event
	EPwm2Regs.ETSEL.bit.INTEN = 1;                  // Enable INT
	EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;             // Generate INT on 1st event   
}

void InitSPwm3()
{
	EPwm3Regs.TBPRD = CarrVal;						// Set timer period
	EPwm3Regs.TBPHS.half.TBPHS = 0x0000;            // Phase is 0
	EPwm3Regs.TBCTR = 0x0000;                       // Clear counter

	EPwm3Regs.CMPA.half.CMPA = 0;					//ϵͳ�ϵ�ʱռ�ձ�Ϊ0

	// Setup TBCLK
	EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 	// Count up
	EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;        	// Disable phase loading
	EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       	// Clock ratio to SYSCLKOUT
	EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;          	// Slow so we can observe on the scope

	// Setup shadowing CMPCTL�������ȽϿ��ƼĴ���
	EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  	// Load on Zero
	EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

	// Set actions
	EPwm3Regs.AQCTLA.bit.CAU = AQ_SET;              // Set PWM1A on CAU
	EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Clear PWM1A on CAD

	EPwm3Regs.AQCTLB.bit.CAU = AQ_CLEAR;            // Clear PWM3B on CAU
	EPwm3Regs.AQCTLB.bit.CAD = AQ_SET;              // Set PWM1B on CAD

	EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
	EPwm3Regs.DBCTL.bit.IN_MODE = DBA_ALL;

	EPwm3Regs.DBRED = 145;
	EPwm3Regs.DBFED = 145;
   
	EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       // Select INT on Zero event
	EPwm3Regs.ETSEL.bit.INTEN = 1;                  // Enable INT
	EPwm3Regs.ETPS.bit.INTPRD = ET_1ST;             // Generate INT on 1st event   
}

//DCDC�����õ�PWM
void InitSPwm4()
{
   //ePWM��ʱ��TBCLK=SYSCLKOUT/(HSPCLKDIV��CLKDIV)
   //PWM�ź�������Ƶ�ʵļ�������
   //Tpwm=(TBPRD+1)*TBCLK
   //Fpwm=1/Tpwm
   EPwm4Regs.TBPRD = DCDC_T;                        // 35.2KHz,DCDC�����õ�PWM��Ƶ��,30MHz=2130,ֵԽ��Ƶ��ԽС
   EPwm4Regs.TBPHS.half.TBPHS = 0x0000;            	// Phase is 0
   EPwm4Regs.TBCTR = 0x0000;                       	// Clear counter

   // Setup compare
   EPwm4Regs.CMPA.half.CMPA = 1995;					//��ʼռ�ձ�5%

   // Setup TBCLK
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 	// Count up
   EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;       	// Disable phase loading
   EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       	// =0,Clock ratio to SYSCLKOUT
   EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;          	// =0,Slow so we can observe on the scope

   // Setup shadowing CMPCTL�������ȽϿ��ƼĴ���
   EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
   EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  	// Load on Zero
   EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

   // Set actions
   EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;              	// Set PWM3A on Zero
   EPwm4Regs.AQCTLA.bit.CAD = AQ_CLEAR;

   EPwm4Regs.AQCTLB.bit.CAU = AQ_CLEAR;            	// Set PWM3A on Zero
   EPwm4Regs.AQCTLB.bit.CAD = AQ_SET;

   // Active high complementary PWMs - Setup the deadband
   EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   //DB_ACTV_HIC PWM���Ϊ����Ч(PWM����ͬʱΪ�͵�ƽ)
   //DB_ACTV_LOC PWM���Ϊ����Ч(PWM����ͬʱΪ�ߵ�ƽ)
   EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;		//=DB_ACTV_HIC(DB_ACTV_HI,DB_ACTV_LOC,DB_ACTV_HIC,DB_ACTV_LO)
   //DBA_ALL PWM����ģʽ
   //DBB_RED_DBA_FED�ǻ���ģʽ����ƽͬ��ͬ��
   EPwm4Regs.DBCTL.bit.IN_MODE = DBA_ALL;			//=DBA_ALL(DBA_ALL,DBB_RED_DBA_FED,DBA_RED_DBB_FED,DBB_ALL)

   //DBRED PWM�������ֵ
   //DBFED PWM�ұ�����ֵ
   EPwm4Regs.DBRED = DCDC_LED;
   EPwm4Regs.DBFED = DCDC_RED;

   // Interrupt where we will change the deadband
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;       	// Select INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = 1;                  	// Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_3RD;             	// Generate INT on 3rd event
}

//��ʼ��ADCģ��
void ADCInit(void)
{
	// ����ADC
	AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;			// ADCTRL1 ADC ���ƼĴ���1
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;        		// ����ģʽ
	AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;		// ADCTRL3 ADC ���ƼĴ���3
	AdcRegs.ADCTRL3.bit.SMODE_SEL= 0;				// ����˳�����ģʽ
	AdcRegs.ADCMAXCONV.all = 0x000F;       			// ADCMAXCONV ADC ���ת���ŵ����Ĵ���,����Ϊ16�ŵ�

	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0; 			// Setup ADCINA0 as 1st SEQ1 conv.ADCCHSELSEQ1  ADC �ŵ�ѡ������ƼĴ���1
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x1; 			// Setup ADCINA1 as 2nd SEQ1 conv.
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 0x2; 			// Setup ADCINA2 as 3nd SEQ1 conv.
	AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 0x3; 			// Setup ADCINA3 as 4nd SEQ1 conv.

	AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 0x4; 			// Setup ADCINA4 as 1st SEQ2 conv.ADCCHSELSEQ2  ADC �ŵ�ѡ������ƼĴ���2
	AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 0x5; 			// Setup ADCINA5 as 2nd SEQ2 conv.
	AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 0x6; 			// Setup ADCINA6 as 3nd SEQ2 conv.
	AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 0x7; 			// Setup ADCINA7 as 4nd SEQ2 conv.

	AdcRegs.ADCCHSELSEQ3.bit.CONV08 = 0x8; 			// Setup ADCINB0 as 1nd SEQ3 conv.ADCCHSELSEQ3  ADC �ŵ�ѡ������ƼĴ���3
	AdcRegs.ADCCHSELSEQ3.bit.CONV09 = 0x9; 			// Setup ADCINB1 as 2nd SEQ3 conv.
	AdcRegs.ADCCHSELSEQ3.bit.CONV10 = 0xA; 			// Setup ADCINB2 as 3nd SEQ3 conv.
	AdcRegs.ADCCHSELSEQ3.bit.CONV11 = 0xB; 			// Setup ADCINB3 as 4nd SEQ3 conv.

	AdcRegs.ADCCHSELSEQ4.bit.CONV12 = 0xC; 			// Setup ADCINB4 as 1nd SEQ4 conv.ADCCHSELSEQ4  ADC �ŵ�ѡ������ƼĴ���3
	AdcRegs.ADCCHSELSEQ4.bit.CONV13 = 0xD; 			// Setup ADCINB5 as 2nd SEQ4 conv.
	AdcRegs.ADCCHSELSEQ4.bit.CONV14 = 0xE; 			// Setup ADCINB6 as 3nd SEQ4 conv.
	AdcRegs.ADCCHSELSEQ4.bit.CONV15 = 0xF; 			// Setup ADCINB7 as 4nd SEQ4 conv.

	AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;			// Enable SOCA from ePWM to start SEQ1 ʹ��ePWMԴ�������
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  			// Enable SEQ1 interrupt (every EOS)

///*
	//ʹ��ePWMԴ��������������
	//Assumes(����) ePWM1 clock is already enabled in InitSysCtrl();
	//Ƭ�ϵ�ADC����EPWMģ��������ͨ������EPWMģ����������ı�ADC�����ʣ�ADC������Ϊ6.4kHz��
	//��F28035��һ����F28335����ֱ��ָ��һ·PWM������������·PWM�Ͳ�������������
	EPwm5Regs.ETSEL.bit.SOCAEN = 1;        			// ETSEL�¼�������ѡ��Ĵ���,Enable SOC on A group
	EPwm5Regs.ETSEL.bit.SOCASEL = 4;       			// Select SOC from from CPMA on upcount
	EPwm5Regs.ETPS.bit.SOCAPRD = 1;        			// ETPS�¼�������Ԥ��Ƶ�Ĵ���
	EPwm5Regs.CMPA.half.CMPA = 0x0080;	  			// CMPA�������Ƚ�A �Ĵ���,Set compare A value
	EPwm5Regs.TBPRD = 0x3A98;              			// Set period for ePWM2
	EPwm5Regs.TBCTL.bit.CTRMODE = 0;		  		// TBCTLʱ�����ƼĴ���,count up and start
//*/
}

//==========================================================================================================
//��ʼ��SPI����
void spi_init()
{    
	SpiaRegs.SPICCR.all =0x004F;		// SPI�����λ, ����λΪ1���½��ط������ݣ�, ÿ���ƽ����Ƴ�16λ�ֳ��ȣ���ֹSPI�ڲ����ͣ�LOOKBACK�����ܣ�
	SpiaRegs.SPICTL.all =0x0006; 		// ʹ������ģʽ��������λ��ʹ���������ͣ���ֹ��������жϣ���ֹSPI�жϣ�

	SpiaRegs.SPIBRR =0x007F;			// SPI������=25M/128	=195.3KHZ��							
    SpiaRegs.SPICCR.all =0x00CF;		// ֹͣSPI�����λ׼�����ջ��ͣ���ֹ����ģʽ�� 
    SpiaRegs.SPIPRI.bit.FREE = 1;  		// ��������     
}

//��ʼ��SPI FIFO
void spi_fifo_init()										
{
    SpiaRegs.SPIFFTX.all=0xE040;		//ʹ��FIFO;��������жϱ�־λ����ֹFIFO�����жϣ������жϼ�����Ϊ0��
    SpiaRegs.SPIFFRX.all=0x204f;		//���FF�����־λ�������������жϱ�־λ����ֹFF�����жϣ������жϼ���Ϊ16��
    SpiaRegs.SPIFFCT.all=0x0;			//SPITXBUF����λ�Ĵ������Ͳ��ӳ٣�
}

//����SPI����
void spi_xmit(Uint16 a)
{
    SpiaRegs.SPITXBUF=a;
}

//==========================================================================================================
//SYS
//��ʱ����
void Delay(Uint16 i)
{
	while(i--);
}

//==========================================================================================================
//                                              ϵͳ������ end
//==========================================================================================================
