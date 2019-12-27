//==========================================================================================================
//                                          IO�ڼ����ܶ������
//==========================================================================================================
#define mERR GpioDataRegs.GPADAT.bit.GPIO12								//PWM1 IGBT����

//PWM1,2,3�ı�����λ����
#define mOC_RESET_PRO_H() {GpioDataRegs.GPASET.bit.GPIO10 = 1;}			//������λ���������=H������ź�
#define mOC_RESET_PRO_L() {GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;}		//������λ�����������λ=L������ź�

#define		JDQ1_ON()	{GpioDataRegs.GPASET.bit.GPIO15 = 1;}
#define		JDQ1_OFF()	{GpioDataRegs.GPACLEAR.bit.GPIO15 = 1;}

#define mRun_ON()	{GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;}				//GPIO13 ��0
#define mRun_OFF()	{GpioDataRegs.GPASET.bit.GPIO13 = 1;}				//GPIO13 ��1

#define mErr_ON()	{GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;}				//GPIO23 ��0
#define mErr_OFF()	{GpioDataRegs.GPASET.bit.GPIO23 = 1;}				//GPIO23 ��1

#define	mPWM_ON()	{EALLOW; \
						EPwm4Regs.TZCLR.bit.OST = 1; \
						EDIS;}
								 
#define	mPWM_OFF()	{EALLOW; \
						EPwm4Regs.TZFRC.bit.OST = 1; \
						EDIS;}

#define	mSPWM_ON()	{EALLOW; \
						EPwm1Regs.TZCLR.bit.OST = 1; \
						EPwm2Regs.TZCLR.bit.OST = 1; \
						EPwm3Regs.TZCLR.bit.OST = 1; \
						EPwm1Regs.TZCLR.bit.INT = 1; \
						EPwm2Regs.TZCLR.bit.INT = 1; \
						EPwm3Regs.TZCLR.bit.INT = 1; \
						EDIS;}
								 
#define	mSPWM_OFF()	{EALLOW; \
						EPwm1Regs.TZFRC.bit.OST = 1; \
						EPwm2Regs.TZFRC.bit.OST = 1; \
						EPwm3Regs.TZFRC.bit.OST = 1; \
						EDIS;}

#define	mSPWMA_ERR()	{EALLOW; \
							EPwm1Regs.TZCLR.bit.OST = 1; \
							EPwm2Regs.TZFRC.bit.OST = 1; \
							EPwm3Regs.TZFRC.bit.OST = 1; \
							EDIS;}






#define KEY1 GpioDataRegs.GPBDAT.bit.GPIO58
#define KEY2 GpioDataRegs.GPBDAT.bit.GPIO59
#define KEY3 GpioDataRegs.GPBDAT.bit.GPIO48

void GPIOInit(void)
{
	EALLOW;

	//mOC_RESET_PRO GPIO10 ����
	GpioCtrlRegs.GPAPUD.bit.GPIO10=0;			//���� GPIO10 ������
	GpioCtrlRegs.GPAMUX1.bit.GPIO10=0;			//GPIO10 ����ΪIO��
	GpioCtrlRegs.GPADIR.bit.GPIO10=1;			//GPIO10 ����Ϊ���
	GpioDataRegs.GPASET.bit.GPIO10 = 1;			//GPIO10 ��1

	//mPWM GPIO15 ����
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;			//���� GPIO15 ������
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;		//GPIO15 ����ΪIO��
	GpioCtrlRegs.GPADIR.bit.GPIO15=1;			//GPIO15 ����Ϊ���
	GpioDataRegs.GPACLEAR.bit.GPIO15 = 1;		//GPIO15 ��0

	//mRun GPIO60 ����
	GpioCtrlRegs.GPAPUD.bit.GPIO13=0;			//���� GPIO13 ������
	GpioCtrlRegs.GPAMUX1.bit.GPIO13=0;			//GPIO13 ����ΪIO��
	GpioCtrlRegs.GPADIR.bit.GPIO13=1;			//GPIO13 ����Ϊ���
	GpioDataRegs.GPASET.bit.GPIO13 = 1;			//GPIO13 ��1

	//mErr GPIO23 ����
	GpioCtrlRegs.GPAPUD.bit.GPIO23=0;			//���� GPIO23 ������
	GpioCtrlRegs.GPAMUX2.bit.GPIO23=0;			//GPIO23 ����ΪIO��
	GpioCtrlRegs.GPADIR.bit.GPIO23=1;			//GPIO23 ����Ϊ���
	GpioDataRegs.GPASET.bit.GPIO23 = 1;			//GPIO23 ��1

	//KEY1 IO����
	GpioCtrlRegs.GPBPUD.bit.GPIO58=0;			//���� GPIO49 ������
	GpioCtrlRegs.GPBMUX2.bit.GPIO58=0;			//GPIO49 ����ΪIO��
	GpioCtrlRegs.GPBDIR.bit.GPIO58=0;			//GPIO49 ����Ϊ����
	//KEY2 IO����
	GpioCtrlRegs.GPBPUD.bit.GPIO59=0;			//���� GPIO59 ������
	GpioCtrlRegs.GPBMUX2.bit.GPIO59=0;			//GPIO59 ����ΪIO��
	GpioCtrlRegs.GPBDIR.bit.GPIO59=0;			//GPIO59 ����Ϊ����
	//KEY3 IO����
	GpioCtrlRegs.GPBPUD.bit.GPIO48=0;			//���� GPIO48 ������
	GpioCtrlRegs.GPBMUX2.bit.GPIO48=0;			//GPIO48 ����ΪIO��
	GpioCtrlRegs.GPBDIR.bit.GPIO48=0;			//GPIO48 ����Ϊ����

	//mERR IO����
	GpioCtrlRegs.GPAPUD.bit.GPIO12=0;			//���� GPIO12 ������
	GpioCtrlRegs.GPAMUX1.bit.GPIO12=0;			//GPIO12 ����ΪIO��
	GpioCtrlRegs.GPADIR.bit.GPIO12=0;			//GPIO12 ����Ϊ����

	EDIS;
}
//==========================================================================================================
//                                          IO�ڼ�LCD���ܶ������ END
//==========================================================================================================
