/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          key.c
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

#include "key.h"
#include "board.h"

/*****************************************************************************
 *初始化按键，长按可开关机
 *使用GPIO1_10作为计数器的输入；
 ****************************************************************************/
void key_init()
{
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_KEYINT_PORT, GPIO_KEYINT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_KEYPIN_ID,
                        (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGMODE_EN));
}

//end of the file
