/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          adc.h
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef  ADC_EXT_DEF
#define ADC_EXT   
#else
#define ADC_EXT   extern
#endif

ADC_EXT void ADC_init(void);

#endif
//end of the file
