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
 *��ʼ��TIMER16_0����Ϊ������ʹ�ã�
 *ʹ��GPIO0_2��Ϊ�����������룻
 ****************************************************************************/ 
void CounterInit(void)
{
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC2|IOCON_MODE_PULLUP));
    LPC_SYSCTL->SYSAHBCLKCTRL |= (1 << 7);                                /* �򿪶�ʱ��ģ��               */
    LPC_TIMER16_0->CTCR = (0x02) |                                        /* ������ģʽ,�½��ز���        */
                          (0x00 << 2);                                    /* ѡ��CAP0                     */
    LPC_TIMER16_0->TC  = 0;                                               /* ��ʼ��������ֵΪ0            */
    LPC_TIMER16_0->TCR = 0x01;                                            /* ����������                   */
}

/*****************************************************************************
 * timer32_0��ʼ����
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
 * ÿ1000ms�����
   ÿ�������
 * ��ȡһ��ADֵ��
 * ��ȡ���ȵ�ת�٣����ȵ�ת��ͨ��LPC_TIMER16_0�ļ���������ȡ��
 * �����ȵ�ת���Ƿ��ں��ʵķ�Χ��
 .
 .����һ��A1(�ο���Դ��������ͨѶЭ��)
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

        //������
        CheckFan();
        
        // KeyBoard LED blinking 
        // ����ʱ���𣬷��Ȼ��Դ����ʱ��˸
        // ����δ����ʱ��˸����������
        if ((!PowerOnFlag) && (!PowerDownFlag))
        {
            PowerLED_Toggle();
        }
        
        //Run LED blinking 
        //���粻��˸
        if(!PowerDownFlag)
        {
            RunLED_Toggle(); 
        }				
    }
}

/*****************************************************************************
 * timer32_1��ʼ����
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
    Chip_TIMER_SetMatch(LPC_TIMER32_1, 1, (timerFreq / TICKRATE_32_1));   //ÿ�������TICKRATE_32_1��
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, 1);
    Chip_TIMER_Enable(LPC_TIMER32_1);

    /* Enable timer interrupt */
    NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
    NVIC_EnableIRQ(TIMER_32_1_IRQn);
}

/*****************************************************************************
 * ÿ50ms�������
 * ÿ���������Ϊ�����������߹ػ��Ļ�׼��
 * ������ڵ���״̬��������û�дӵ���״̬�ָ����ӵ�״̬��
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
                //��⵽�ϵ�
                PowerDownFlag = 0;  
                DCB_ERR1[7]=DCB_ERR1[7]&(~(1<<0));	
                
                PowerDownErrFlag = 0;
                
                //�ϵ�����¼����Ⱥ͵�Դ��ѹ
                FanErrCnt = 0;
                FanErrFlag = 0;
                BusVoltageErrCnt = 0;
                BusVoltageErrFlag = 0; 
                FanErrLED_OFF();
                VoltErrLED_OFF();
                //���ÿ��Ź�
                //wdt_reset(); 
                //wdt_disable();
            }
        }
    }
}

//end of the file
