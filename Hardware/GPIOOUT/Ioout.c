/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          ioout.c
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

#include "Ioout.h"
#include "delay.h"

extern volatile unsigned char AlarmFlag;                        //正在报警的时候不关闭蜂鸣器
extern volatile unsigned char PowerDownFlag;                    //Power down accidentally
extern unsigned char FanSpeedMax;                               // normally, fan speed=3400 rpm,  2 pules/revolution, 113 pules each second
extern unsigned char FanSpeedMin;
extern volatile unsigned char FanErrFlag, FanErrCnt;            //Fan speed exceed limits
extern volatile unsigned char PowerOnFlag;	
extern volatile unsigned char AdcFlag;
extern unsigned char BusVoltage;
extern unsigned char BusVoltageMax;                             //  100 = 20 V,  180 = 28 v
extern unsigned char BusVoltageMin;
extern unsigned char BusVoltageErrFlag, BusVoltageErrCnt;       //Main 24V DC supply exceed limits
extern volatile unsigned char KeyScanFlag;   
extern unsigned char LastKey;
extern unsigned char CurretKey;
extern unsigned char KeyCnt;
extern unsigned char HoldCnt;
extern unsigned char AlarmCnt;                                  //掉电报警次数
extern unsigned char DCB_ERR1[8];   
extern unsigned short Fan_speed;
extern unsigned char  R_Selfcheck;

//
// AlarmBeep: n short beep 
//
void AlarmBeep(unsigned char n)
{
    unsigned char i;
    
    BUZZER_OFF();
    myDelay(700);

    AlarmFlag = 1;
    for (i=0; i<n; i++)
    {
        BUZZER_ON();
        myDelay(100);
        BUZZER_OFF();
        myDelay(100);
    }
    AlarmFlag = 0;
}

//
// Check Fan error
//
void CheckFan(void)
{
    //正常状态(未掉电),检测风扇错误
    if(!PowerDownFlag)
    {
        if((Fan_speed>FanSpeedMin)&&(Fan_speed<FanSpeedMax))
        { 
            if(FanErrCnt > 0)
            {
                FanErrCnt--;
            }
            else
            {   
                FanErrFlag = 0;
                DCB_ERR1[7]=DCB_ERR1[7]&(~(1<<2));
                FanErrLED_OFF();
            }
        }
        else
        { 
            if(FanErrCnt < FAN_ERR_TIME)
            {
                FanErrCnt ++;
            }
            else
            {    
                FanErrFlag = 1;   
                R_Selfcheck=0;                
                DCB_ERR1[7]=DCB_ERR1[7]|(1<<2);
                FanErrLED_ON();
            }
        }
    }
}

//
// Check Bus Volt
//
void CheckBusVolt(void)
{
    uint16_t dataADC;
    if((PowerOnFlag)&&(AdcFlag))
    {
        AdcFlag = 0;
        
        Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
        while (Chip_ADC_ReadStatus(LPC_ADC, ADC_CH0, ADC_DR_DONE_STAT) != SET) {}
        Chip_ADC_ReadValue(LPC_ADC, ADC_CH0, &dataADC);	
        //ADCSRA = 0XC4; 
        
        BusVoltage=(((dataADC>>2)&0xff)*14.2/10)-100;
        if ((BusVoltage > BusVoltageMin) && (BusVoltage < BusVoltageMax))
        {
            if(BusVoltageErrCnt > 1)
            {
                BusVoltageErrCnt--;
            }
            else
            {
                BusVoltageErrFlag = 0;
                DCB_ERR1[7]=DCB_ERR1[7]&(~(1<<1));
                VoltErrLED_OFF();
            }
        }
        else //if(!PowerOnFlag)
        {
            if(BusVoltageErrCnt < BUSVOT_REE_TIME)
            {
                BusVoltageErrCnt++;
            }
            else
            {
                //BusVoltageErrFlag=0;
                BusVoltageErrFlag = 1;  
                R_Selfcheck=0;
                DCB_ERR1[7]=DCB_ERR1[7]|(1<<1);
                VoltErrLED_ON();
            }
        } 
    }
}

/*****************************************************************************
 * 按键检测（开关机）
 * 长按1S开机
 * 长按3S关机
 ****************************************************************************/
void Button_Detect(void)
{
    unsigned char temp = 0;
    if(KeyScanFlag)
    {
        KeyScanFlag = 0;

        // 未上电不扫描键盘
        // 没开机时风扇或电源电压有误也扫描键盘
        // 开机后发现风扇或电源电压有误，允许按键关机
        if(PowerDownFlag == 0)
        {
            //使用GPIO1_10作为按键的引脚
            temp = Chip_GPIO_ReadPortBit(LPC_GPIO, 1, 10);
            LastKey = CurretKey;
            CurretKey = temp;

            if(CurretKey == 0)
            {
                if(LastKey == 0)
                {
                    if(HoldCnt ==  0)
                    {
                        BUZZER_ON();
                        KeyCnt ++;
                        if(PowerOnFlag==1)
                        {
                            //To turn DC/DC and  Main Power(24V)  Off
                            if(KeyCnt > KEY_PowerOFF_TIME)
                            {
                                BUZZER_OFF();
                                DCModuleCtl_OFF();
                                myDelay(1500);
                                MainPowerCtl_OFF();
                                KeyCnt = 0;
                                HoldCnt = 1;
                                PowerOnFlag = 0;
                            }
                        }
                        else
                        {
                            //To turn  Main Power(24V)  and DC/DC on
                            if(KeyCnt > KEY_PowerON_TIME)
                            {            
                                BUZZER_OFF(); 
                                if ((!FanErrFlag) && (!BusVoltageErrFlag ))
                                {
                                    MainPowerCtl_ON();
                                    myDelay(1500);
                                    DCModuleCtl_ON();
                                    KeyCnt = 0;
                                    HoldCnt = 1;
                                    AlarmCnt = 0;
                                    PowerOnFlag  = 1;
                                    PowerLED_ON();
                                }
                                else if(BusVoltageErrFlag)
                                {
                                    AlarmBeep(3);
                                }  
                                else if(FanErrFlag)
                                {
                                    AlarmBeep(4);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                //?????????????
                if(!AlarmFlag)
                {
                    BUZZER_OFF();
                }

                if(LastKey == 0)
                {
                    HoldCnt = 0;
                    KeyCnt = 0;
                }
            }
        }
    }
}

/*****************************************************************************
 * 初始化单片机用作输出的IO口
 ****************************************************************************/
void Ioout_init(void)
{
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 3);      //蜂鸣器
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 0);      //电源电压错误指示灯    
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 1);      //风扇错误指示灯
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 7);      //面板指示灯

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 3, 2);      //主24V输出；
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 9);      //DC-DC
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 2, 10);     //程序运行指示灯
}

/*****************************************************************************
 * 蜂鸣器 打开
 ****************************************************************************/
void BUZZER_ON(void)
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 3, true);       //output  high, SOUND
}
/*****************************************************************************
 * 蜂鸣器 关闭
 ****************************************************************************/
void BUZZER_OFF(void)       
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 3, false);       //output  low,  MUTE
}

/*****************************************************************************
 * 运行指示灯 关闭
 ****************************************************************************/

void RunLED_OFF(void)        
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 10, true);       //output  high, Run LED OFF
}

/*****************************************************************************
 * 运行指示灯 打开
 ****************************************************************************/
void  RunLED_ON(void)    
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 10, false);         //output  low,  Run LED LIGHT
}

/*****************************************************************************
 * 运行指示灯 闪烁
 ****************************************************************************/
void  RunLED_Toggle(void)      
{
    bool state;
    state= Chip_GPIO_ReadPortBit(LPC_GPIO, 2, 10);
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 10, state^1);
}

/*****************************************************************************
 * 风扇错误指示灯 关闭
 ****************************************************************************/
void  FanErrLED_OFF(void)	    
{
   	Chip_GPIO_SetPinState(LPC_GPIO, 3, 1, true);     //output  high, Fan err LED OFF
}

/*****************************************************************************
 * 风扇错误指示灯 打开
 ****************************************************************************/
void  FanErrLED_ON(void)   
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 1, false);     //output  low,  Fan err LED LIGHT
}
 
/*****************************************************************************
 * 电源电压错误指示灯 关闭
 ****************************************************************************/
void VoltErrLED_OFF(void)    
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 0, true);    //output  high, Voltage err LED OFF
}
/*****************************************************************************
 * 电源电压错误指示灯 打开
 ****************************************************************************/
void VoltErrLED_ON(void)    
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 0, false);    //output  low,  Voltage err LED LIGHT
}
 
 /*****************************************************************************
 * 面板指示灯 打开
 ****************************************************************************/   
void PowerLED_ON(void)     
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 7, false);      //output  high, Panel LED LIGHT
}

 /*****************************************************************************
 * 面板指示灯 关闭
 ****************************************************************************/ 
void PowerLED_OFF(void)      
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 7, true);     //output  low,  Panel LED OFF
}

 /*****************************************************************************
 * 面板指示灯 闪烁
 ****************************************************************************/ 
void PowerLED_Toggle()   
{
    bool state;
    state= Chip_GPIO_ReadPortBit(LPC_GPIO, 2, 7);
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 7, state^1);    
}

 /*****************************************************************************
 * DC-DC电压转换  关闭
 ****************************************************************************/ 
void DCModuleCtl_OFF(void)	  
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 9, true);   //output  high, DC/DC OFF
}
 /*****************************************************************************
 * DC-DC电压转换  打开
 ****************************************************************************/ 
void DCModuleCtl_ON(void)   
{
    Chip_GPIO_SetPinState(LPC_GPIO, 2, 9, false);   //output  low,  DC/DC ON
}

 /*****************************************************************************
 * 主电源继电器  打开
 ****************************************************************************/ 
void MainPowerCtl_ON(void)	 
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 2, true);  //output  high, Main 24V Power ON
}

 /*****************************************************************************
 * 主电源继电器  关闭
 ****************************************************************************/ 
void MainPowerCtl_OFF(void)  
{
    Chip_GPIO_SetPinState(LPC_GPIO, 3, 2, false);  //output  low,  Main 24V Power OFF
}

 /*****************************************************************************
 * 电源板自检
//自检结果各位的定义
#define  STATE_NOERR                     00          // 没有错误
#define  STATE_ONCKECKING                01          // 自检中
#define  STATE_SOMEERRS                  02          // 自检没有通过
#define  STATE_HAVENOTCHECK              03          // 还没有自检
#define  STATE_POWERBELOWSETTING        (1<<2)       // 电源电源低于设定值
#define  STATE_POWERUNDERSETTING        (1<<3)       // 电源电压高于设定值
#define  STATE_FANSPEEDLOW              (1<<4)       // 风扇速度低于设定值
#define  STATE_FANSPEEDHIGH             (1<<5)       // 风扇速度高于设定值
 ****************************************************************************/ 
unsigned char HardwareSelfTest(void)
{
    unsigned char temp_err=0,temp_flag=0,temp_err_result=0;
    if (BusVoltage < BusVoltageMin)
    {
        temp_err |= STATE_POWERBELOWSETTING;
        temp_flag=1;
    }
    if (BusVoltage > BusVoltageMax)
    {
        temp_err |= STATE_POWERUNDERSETTING;
        temp_flag=1;
    }
    if(Fan_speed<FanSpeedMin)
    {
        temp_err |= STATE_FANSPEEDLOW;
        temp_flag=1;
    }
    if(Fan_speed>FanSpeedMax)
    {
        temp_err |= STATE_FANSPEEDHIGH;
        temp_flag=1;
    }
    if(temp_flag==1)
    {
        temp_err_result=STATE_SOMEERRS|temp_err;
    }
    else
    {
        temp_err_result=STATE_NOERR;
    }
        return temp_err_result;
}

// end of the file
