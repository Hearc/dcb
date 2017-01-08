/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          timer.h
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
    
#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef  TIMER_EXT_DEF
#define TIMER_EXT   
#else
#define TIMER_EXT   extern
#endif

#include "board.h"
#include "exint.h"

#define TICKRATE_32_0       (1)                   /* TICKRATE_32_0=1，每秒钟溢出1次 */
#define TICKRATE_32_1       (20)                  /*  * TICKRATE_32_1=20,每秒钟中断20次 */


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

TIMER_EXT void timer32_0_init(void);
TIMER_EXT void timer32_1_init(void);
TIMER_EXT void CounterInit(void);

#endif
//end of the file
