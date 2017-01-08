/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          main.c
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

//#include "board.h"
#include "uart.h"
#include "chip.h"
#include "timer.h"
#include "Ioout.h"
#include "adc.h"
#include "exint.h"
#include "key.h"
#include "delay.h"
#include "wdg.h"

///////////////////////////////////////////////////////////////////////////

volatile unsigned char KeyScanFlag = 0;           
volatile unsigned char AdcFlag = 0;

//掉电标识位  1：掉电   0： 没有掉电
volatile unsigned char PowerDownFlag = 1;                   //Power down accidentally

//掉电错误标识位  1：有错误   0： 没有错误
volatile unsigned char PowerDownErrFlag = 0;                //Power down when PowerOn , Error BEEP!

//开机标识位 1：已开机  0： 关机，或者24V掉电
volatile unsigned char PowerOnFlag = 0;	

//风扇错误的次数超过设定的次数标识位   风扇错误次数
volatile unsigned char FanErrFlag = 0, FanErrCnt = 0;       //Fan speed exceed limits

//电源掉电次数超过设定的次数标识位，电源电压超出范围次数
unsigned char BusVoltageErrFlag = 0, BusVoltageErrCnt = 0;  //Main 24V DC supply exceed limits

unsigned int FanSpeedMax = 150;    // normally, fan speed=3400 rpm,  2 pules/revolution, 113 pules each second
unsigned int FanSpeedMin = 60;

unsigned int BusVoltageMax = 160;  //  100 = 20 V,  180 = 28 v
unsigned int BusVoltageMin = 120;

//上次按键值
unsigned char LastKey;
//本次按键值
unsigned char CurretKey;
//记录按下的按键次数
unsigned char KeyCnt;
//松手检测
unsigned char HoldCnt;

unsigned char AlarmCnt = 0;                         //掉电报警次数
volatile unsigned char AlarmFlag = 0;               //正在报警的时候不关闭蜂鸣器

unsigned short BusVoltage;                          //电源电压值
volatile unsigned char UartTransmit_Sec = 0;        //每秒钟发送数据标识位，定时器溢出置位，发送完数据清0；
unsigned char Current_State;                        //主机当前状态，0 开机界面  1 启动动画  2系统自检 3称重定标  4准备预冲  5进行预冲 6进行治疗  7确认结束   8治疗结束
                                                    //              9 系统维护  10称重校准  11治疗参数设置   12 系统设置1   13 系统设置2
unsigned char  R_Selfcheck=1;                       //自检结果，  0：自检没有通过 1 ： 自检通过
 
unsigned short Fan_speed;                           //风扇运行速度                         
unsigned char UartTx_Buf[8];                        //单片机向主机发送的数据保存的数组
unsigned char UartReceiveBuf[9];                    //单片接收主机主机发送来的数据保存的数组
unsigned char EndReceive=0;                         //接收完一个数据待执行回复的标志；                  
unsigned char Hard_VER=11,Soft_VER=11;              //硬件版本，软件版本
unsigned int  TotalTimeAPU=0;                       //上电后运行时间
 //自检结果
unsigned short SelfCheckErr=0xffff;  
//启动自检
unsigned char StartSelfTest=0;
 
 //错误标识，  //unsigned long DCB_ERR1
 //BIT0  电源掉电
 //BIT1  电源电压不在要求的范围内((BusVoltage>=22)&&(BusVoltage<=26))
 //BIT2  风扇转速不在要求的范围内((Fan_speed=60)&&(Fan_speed<=150))
 unsigned char DCB_ERR1[8]={0,0,0,0,0,0,0,0};
 
/**

 */
int main(void)
{
    SystemCoreClockUpdate();        //系统时钟初始化
//	Board_Init();           	
    uart_init();                    //串口初始化 8N1
    
    timer32_1_init();               //timer32_1初始化，每50Ms溢出	
    timer32_0_init();               //timer32_0初始化，每1000Ms溢出
    
    CounterInit();                  //timer16初始化，用作计数器，读取风扇转速   
    Ioout_init();                   //初始化需要电源板的lpc11c14控制的设备引脚；
    ADC_init();                     //初始化AD，用于读取计算外部24V是否在所要求的范围；
    exint0_init();                  //外部中断初始化，用于测试外部24V有没有掉电；       
    key_init();                     //按键初始化，按键用于开，关机
    WWDT_init();
    while (1) 
    {
        Button_Detect();            //检测按键
        CheckBusVolt();             //读取电压AD值
        Uart_Sent_Sec();            //每秒钟发送一次A0和A1;                     
        uart_cmd();                 //响应上位机指令

        //治疗状态下，掉电报警2min。
        if ( PowerDownErrFlag && (0x06 == Current_State))                                                         
        {            
            for (AlarmCnt = 0; ((AlarmCnt < 90) && ( PowerDownErrFlag)); AlarmCnt++)
            {
                Chip_WWDT_Start(LPC_WWDT);
                AlarmBeep(2);
            }
            myDelay(3000);
        }
        
        if ( StartSelfTest==1)
        {
           StartSelfTest=0;
           SelfCheckErr=HardwareSelfTest();
        }
    }
}

// end of the file
