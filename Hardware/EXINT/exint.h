/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          exint.h
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

#ifndef __EXINT_H__
#define __EXINT_H__

//#include "board.h"
//#include "Ioout.h"

extern volatile unsigned char PowerDownFlag ; 
extern volatile unsigned char FanErrFlag, FanErrCnt;        //Fan speed exceed limits
extern unsigned char BusVoltageErrFlag, BusVoltageErrCnt;   //Main 24V DC supply exceed limits
extern volatile unsigned char PowerOnFlag;	
extern volatile unsigned char PowerDownErrFlag;             //Power down when PowerOn , Error BEEP!
extern unsigned char KeyCnt;
extern unsigned char HoldCnt;
extern unsigned char AlarmCnt;  
extern unsigned char DCB_ERR1[8]; 
extern unsigned char Current_State; 

//主状态定义
#define STATE_NULL              0       // 无
#define STATE_STARTANIMAL       1       // 启动动画
#define STATE_SELFTEST          2       // 系统自检
#define STATE_WEICALIBRATE      3       // 称重定标
#define STATE_PREPRIMING        4       // 准备预冲
#define STATE_PRIMING           5       // 进行预冲
#define STATE_THERAPY           6       // 进行治疗
#define STATE_FINISHED          7       // 确认结束
#define STATE_FINISH            8       // 治疗结束
#define STATE_MAINTANCE         9       // 系统维护
#define STATE_WEIGHING          0X0A    // 称重校准
#define STATE_PARAMSET          0X0B    // 治疗参数设置
#define STATE_SETTING1          0X0C    // 系统设置1
#define STATE_SETTING2          0X0D    // 系统设置2

void exint0_init(void);

#endif
//end of the file
