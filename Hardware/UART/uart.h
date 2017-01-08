/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          uart.h
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

#ifndef __UART_H__
#define	__UART_H__


#include "stdlib.h"
#include "board.h"

extern uint8_t UartReceiveBuf[9];
extern uint8_t EndReceive;

void uart_init(void);
void uart_cmd(void);
void Uart_Sent_Sec(void);

#endif
//end of the file
