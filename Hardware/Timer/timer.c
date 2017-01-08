/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          timer.c
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

#define  TIMER_EXT_DEF

#include "timer.h"
#include "Ioout.h"

extern volatile unsigned char AdcFlag;
extern volatile unsigned char PowerOnFlag;  
extern volatile unsigned char PowerDownFlag; 
extern volatile unsigned char KeyScanFlag;
extern volatile unsigned char PowerDownErrFlag;                 //Power down when PowerOn , Error BEEP!
extern volatile unsigned char FanErrFlag, FanErrCnt;            //Fan speed exceed limits
extern unsigned char BusVoltageErrFlag , BusVoltageErrCnt ;     //Main 24V DC supply exceed limits
extern volatile unsigned char UartTransmit_Sec;  
extern unsigned char DCB_ERR1[8]; 
extern unsigned int TotalTimeAPU;
extern unsigned short Fan_speed;
extern unsigned char Current_State;  

/*****************************************************************************
 *初始化TIMER16_0，作为计数器使用；
 *使用GPIO0_2作为计数器的输入；
 ****************************************************************************/ 
void CounterInit(void)
{
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC2|IOCON_MODE_PULLUP));
    LPC_SYSCTL->SYSAHBCLKCTRL |= (1 << 7);                                /* 打开定时器模块               */
    LPC_TIMER16_0->CTCR = (0x02) |                                        /* 计数器模式,下降沿捕获        */
                          (0x00 << 2);                                    /* 选择CAP0                     */
    LPC_TIMER16_0->TC  = 0;                                               /* 初始化计数器值为0            */
    LPC_TIMER16_0->TCR = 0x01;                                            /* 启动计数器                   */
}

/*****************************************************************************
 * timer32_0初始化，
 ****************************************************************************/
void timer32_0_init(void)
{
    uint32_t timerFreq;
    /* Enable and setup SysTick Timer at a periodic rate */
    // SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

    /* Enable timer 0 clock */
    Chip_TIMER_Init(LPC_TIMER32_0);     

    /* Timer rate is system clock rate */
    timerFreq = Chip_Clock_GetSystemClockRate();

    /* Timer setup for match and interrupt at TICKRATE_HZ */
    Chip_TIMER_Reset(LPC_TIMER32_0);
    Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
    Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, (timerFreq / TICKRATE_32_0));
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 1);
    Chip_TIMER_Enable(LPC_TIMER32_0);

    /* Enable timer interrupt */
    NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
    NVIC_EnableIRQ(TIMER_32_0_IRQn);
}

//void SysTick_Handler(void)
//{
//	Board_LED_Set(0, false);
//}

/*****************************************************************************
 * 每1000ms钟溢出
   每次溢出：
 * 读取一次AD值；
 * 读取风扇的转速，风扇的转速通过LPC_TIMER16_0的计数器来读取；
 * 检测风扇的转速是否在合适的范围；
 .
 .发送一次A1(参考电源板与主机通讯协议)
 ****************************************************************************/
void TIMER32_0_IRQHandler(void)
{
    if(Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) 
    {
        Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
        //start Main 24V supply Voltage measurement each 1000ms
        AdcFlag = 1; 
        UartTransmit_Sec=1;
        TotalTimeAPU++;
        //Fan speed detect
        Fan_speed = LPC_TIMER16_0->TC;     //TWI_buf[2]: Fan speed
        LPC_TIMER16_0->TC = 0;

        //检测风扇
        CheckFan();
        
        // KeyBoard LED blinking 
        // 掉电时不灭，风扇或电源有误时闪烁
        // 无误未开机时闪烁，开机后亮
        if ((!PowerOnFlag) && (!PowerDownFlag))
        {
            PowerLED_Toggle();
        }
        
        //Run LED blinking 
        //掉电不闪烁
        if(!PowerDownFlag)
        {
            RunLED_Toggle(); 
        }				
    }
}

/*****************************************************************************
 * timer32_1初始化，
 ****************************************************************************/
void timer32_1_init(void)
{
    uint32_t timerFreq;
    /* Enable and setup SysTick Timer at a periodic rate */
    //SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

    /* Enable timer 1 clock */
    Chip_TIMER_Init(LPC_TIMER32_1);     

    /* Timer rate is system clock rate */
    timerFreq = Chip_Clock_GetSystemClockRate();

    /* Timer setup for match and interrupt at TICKRATE_HZ */
    Chip_TIMER_Reset(LPC_TIMER32_1);
    Chip_TIMER_MatchEnableInt(LPC_TIMER32_1, 1);
    Chip_TIMER_SetMatch(LPC_TIMER32_1, 1, (timerFreq / TICKRATE_32_1));   //每秒钟溢出TICKRATE_32_1次
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, 1);
    Chip_TIMER_Enable(LPC_TIMER32_1);

    /* Enable timer interrupt */
    NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
    NVIC_EnableIRQ(TIMER_32_1_IRQn);
}

/*****************************************************************************
 * 每50ms钟溢出；
 * 每次溢出都作为长按开机或者关机的基准；
 * 如果是在掉电状态，则检测有没有从掉电状态恢复到加电状态；
 ****************************************************************************/
void TIMER32_1_IRQHandler(void)
{
    static unsigned char temp = 0;

    if(Chip_TIMER_MatchPending(LPC_TIMER32_1, 1)) 
    {
        Chip_TIMER_ClearMatch(LPC_TIMER32_1, 1);
         
        KeyScanFlag = 1;

        //update PowerDownFlag
        if( PowerDownFlag )
        {   
            temp = Chip_GPIO_ReadPortBit(LPC_GPIO, 0, 3);    //GPIO0_3
            if (temp)  
            {
                //检测到上电
                PowerDownFlag = 0;  
                DCB_ERR1[7]=DCB_ERR1[7]&(~(1<<0));	
                
                PowerDownErrFlag = 0;
                
                //上电后重新检测风扇和电源电压
                FanErrCnt = 0;
                FanErrFlag = 0;
                BusVoltageErrCnt = 0;
                BusVoltageErrFlag = 0; 
                FanErrLED_OFF();
                VoltErrLED_OFF();
                //禁用看门狗
                //wdt_reset(); 
                //wdt_disable();
            }
        }
    }
}

//end of the file
