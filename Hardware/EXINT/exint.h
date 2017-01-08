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

//��״̬����
#define STATE_NULL              0       // ��
#define STATE_STARTANIMAL       1       // ��������
#define STATE_SELFTEST          2       // ϵͳ�Լ�
#define STATE_WEICALIBRATE      3       // ���ض���
#define STATE_PREPRIMING        4       // ׼��Ԥ��
#define STATE_PRIMING           5       // ����Ԥ��
#define STATE_THERAPY           6       // ��������
#define STATE_FINISHED          7       // ȷ�Ͻ���
#define STATE_FINISH            8       // ���ƽ���
#define STATE_MAINTANCE         9       // ϵͳά��
#define STATE_WEIGHING          0X0A    // ����У׼
#define STATE_PARAMSET          0X0B    // ���Ʋ�������
#define STATE_SETTING1          0X0C    // ϵͳ����1
#define STATE_SETTING2          0X0D    // ϵͳ����2

void exint0_init(void);

#endif
//end of the file
