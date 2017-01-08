/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          wdg.c
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

#include "wdg.h"
#include "board.h"

//初始化看门狗
//开机时不使能看门狗，电源掉电后才使能
void WWDT_init(void)
{
    uint32_t wdtFreq;
    /* Initialize WWDT (also enables WWDT clock) */
    Chip_WWDT_Init(LPC_WWDT);                                            //禁用WDG，   设置溢出时间

    /* Prior to initializing the watchdog driver, the clocking for the
    watchdog must be enabled. This example uses the watchdog oscillator
    set at a 50KHz (1Mhz / 20) clock rate. */
    Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_WDTOSC_PD);                     //  
    Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 20);                           //   1.05Mhz
    
    /* The WDT divides the input frequency into it by 4 */
    wdtFreq = Chip_Clock_GetWDTOSCRate() / 4;
    
    Chip_Clock_SetWDTClockSource(SYSCTL_WDTCLKSRC_WDTOSC, 1);
    
    Chip_WWDT_SetTimeOut(LPC_WWDT, wdtFreq * 4-2);
    Chip_WWDT_SetOption(LPC_WWDT, WWDT_WDMOD_WDRESET);
    //Chip_WWDT_Start(LPC_WWDT);
}

//end of the file
