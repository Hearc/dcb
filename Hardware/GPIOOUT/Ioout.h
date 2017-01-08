/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          ioout.h
* Compiler :     
* Revision :     Revision
* Date :         2017-01-08
* Updated by :   
* Description :  
*                
* 
* LPC11C14 sytem clock: 48Mhz
* system clock: 48MHz
********************************************************************************
* This edition can only be used in DCBoard V6.0 using MornSun DC/DC Module
*******************************************************************************/

//constant
#define	FAN_ERR_TIME        3   //���ȹ���ȷ�ϴ���
#define	BUSVOT_REE_TIME     3   //��Դ��ѹ����ȷ�ϴ���
#define	KEY_HOLDING_TIME    20
#define	KEY_PRESSED_TIME    2
#define	KEY_PowerON_TIME    20
#define	KEY_PowerOFF_TIME   60
#define FW_VER              62  //Edition  60 = V6.0
    

//�Լ�����λ�Ķ���
#define  STATE_NOERR                     00          // û�д���
#define  STATE_ONCKECKING                01          // �Լ���
#define  STATE_SOMEERRS                  02          // �Լ�û��ͨ��
#define  STATE_HAVENOTCHECK              03          // ��û���Լ�
#define  STATE_POWERBELOWSETTING        (1<<2)       // ��Դ��Դ�����趨ֵ
#define  STATE_POWERUNDERSETTING        (1<<3)       // ��Դ��ѹ�����趨ֵ
#define  STATE_FANSPEEDLOW              (1<<4)       // �����ٶȵ����趨ֵ
#define  STATE_FANSPEEDHIGH             (1<<5)       // �����ٶȸ����趨ֵ


extern volatile unsigned char AlarmFlag;

void BUZZER_ON(void);
void BUZZER_OFF(void);
void RunLED_OFF(void);
void RunLED_ON(void);
void RunLED_Toggle(void);

void FanErrLED_OFF(void);
void FanErrLED_ON(void);
void FanErrLED_Toggle(void);

void VoltErrLED_OFF(void);
void VoltErrLED_ON(void) ;
void VoltErrLED_Toggle(void);
  
void PowerLED_ON(void);
void PowerLED_OFF(void); 
void PowerLED_Toggle(void);

void DCModuleCtl_OFF(void);
void DCModuleCtl_ON(void);

void MainPowerCtl_ON(void); 
void MainPowerCtl_OFF(void);

void Ioout_init(void);
void CheckFan(void);
void AlarmBeep(unsigned char n);
void CheckBusVolt(void);

void Button_Detect(void);

unsigned char HardwareSelfTest(void);

// End of file
