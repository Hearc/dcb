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
#define	FAN_ERR_TIME        3   //风扇故障确认次数
#define	BUSVOT_REE_TIME     3   //电源电压故障确认次数
#define	KEY_HOLDING_TIME    20
#define	KEY_PRESSED_TIME    2
#define	KEY_PowerON_TIME    20
#define	KEY_PowerOFF_TIME   60
#define FW_VER              62  //Edition  60 = V6.0
    

//自检结果各位的定义
#define  STATE_NOERR                     00          // 没有错误
#define  STATE_ONCKECKING                01          // 自检中
#define  STATE_SOMEERRS                  02          // 自检没有通过
#define  STATE_HAVENOTCHECK              03          // 还没有自检
#define  STATE_POWERBELOWSETTING        (1<<2)       // 电源电源低于设定值
#define  STATE_POWERUNDERSETTING        (1<<3)       // 电源电压高于设定值
#define  STATE_FANSPEEDLOW              (1<<4)       // 风扇速度低于设定值
#define  STATE_FANSPEEDHIGH             (1<<5)       // 风扇速度高于设定值


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
