/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          adc.c
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

#define  ADC_EXT_DEF

#include "adc.h"
#include "board.h"
#include "adc_11xx.h"

static ADC_CLOCK_SETUP_T ADCSetup;

/*****************************************************************************
 * 使用GPIO0_11作为AD的输入；
 ****************************************************************************/
void Init_ADC_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 11, FUNC2);
#elif defined(BOARD_NXP_XPRESSO_11C24)
//	Chip_GPIO_SetPinDIRInput(LPC_GPIO,0,11);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_11, FUNC2)  ;          //|IOCON_MODE_INACT|IOCON_ADMODE_EN);
//#else
//	#error "Pin muxing for ADC not configured"
#endif
}

/*****************************************************************************
 * 初始化ADC
 * 使用GPIO0_11作为AD的输入；
 * 参考电压外部3.3V
 * AD为  10  位
 ****************************************************************************/
void ADC_init(void)
{
    Init_ADC_PinMux();
    Chip_ADC_Init(LPC_ADC, &ADCSetup);
    Chip_ADC_EnableChannel(LPC_ADC, ADC_CH0, ENABLE);
    //	Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
}

//end of the file
