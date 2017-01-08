/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          exint.C
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

#include "exint.h"
//#include "pinint.h"
//#include "board.h"
#include "delay.h"
#include "Ioout.h"

/* GPIO pin for GPIO pin interrupt */
#define GPIO_PININT             3   /* GPIO pin number mapped to PININT */
#define GPIO_PININT_PORT        0   /* GPIO port number mapped to PININT */
#define IOCON_PIN_ID            IOCON_PIO0_3            /* IOCON pin identifer */
#define PININT_IRQ_HANDLER      PIOINT0_IRQHandler      /* GPIO interrupt IRQ function name */
#define PININT_NVIC_NAME        EINT0_IRQn              /* GPIO interrupt NVIC interrupt name */

#define PowerDownINT            Chip_GPIO_ReadPortBit(LPC_GPIO, GPIO_PININT_PORT, GPIO_PININT);

//掉电后进入中断
void PININT_IRQ_HANDLER(void)
{
    unsigned int temp;
    /* Clear interrupt */
    Chip_GPIO_ClearInts(LPC_GPIO, GPIO_PININT_PORT, (1 << GPIO_PININT));

    myDelay(10);
    temp = PowerDownINT;
    
    if (!temp)
    {
        PowerDownFlag = 1;
        DCB_ERR1[7]=DCB_ERR1[7]|(1<<0);
        FanErrCnt = 0;
        FanErrFlag = 0;
        BusVoltageErrCnt = 0;
        BusVoltageErrFlag = 0;
        RunLED_ON(); 
        PowerLED_OFF(); 
        FanErrLED_ON();
        VoltErrLED_ON(); 
        
        if(PowerOnFlag)
        {
            PowerDownErrFlag = 1;
            KeyCnt = 0;
            PowerOnFlag = 0;
            HoldCnt = 0;
            AlarmCnt = 0;
            DCModuleCtl_OFF();	
            BUZZER_OFF(); 
            MainPowerCtl_OFF();                    
        }
        
        //启动看门狗
        //_delay_ms(3000);
        //wdt_enable(WDTO_2S);
        //wdt_reset(); 
    }
}

/*****************************************************************************
 *外部中断初始化；
 *下降沿触发
 *使用GPIO0_3作为中断的输入；
 *这里是检测24V的电源是否还在(参考原理图)
 ****************************************************************************/
void exint0_init()
{
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_PININT_PORT, GPIO_PININT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIN_ID,
                        (IOCON_FUNC0 | IOCON_DIGMODE_EN));
    /* Configure channel interrupt as edge sensitive and falling edge interrupt */
    Chip_GPIO_SetPinModeEdge(LPC_GPIO, GPIO_PININT_PORT, (0 << GPIO_PININT));        //IS=0: 边沿触发
    Chip_GPIO_SetEdgeModeSingle(LPC_GPIO, GPIO_PININT_PORT, (0 << GPIO_PININT));     //0：由GPIO0IEV来控制中断方式
    Chip_GPIO_SetModeLow(LPC_GPIO, GPIO_PININT_PORT, (0 << GPIO_PININT));            //0：下降沿或者是低电平触发

    /* Enable GPIO pin intrerrupt */
    Chip_GPIO_EnableInt(LPC_GPIO, GPIO_PININT_PORT, (1 << GPIO_PININT));
    NVIC_EnableIRQ(PININT_NVIC_NAME);
}

//end of the file
