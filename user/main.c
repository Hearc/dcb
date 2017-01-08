/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          main.c
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

//#include "board.h"
#include "uart.h"
#include "chip.h"
#include "timer.h"
#include "Ioout.h"
#include "adc.h"
#include "exint.h"
#include "key.h"
#include "delay.h"
#include "wdg.h"

///////////////////////////////////////////////////////////////////////////

volatile unsigned char KeyScanFlag = 0;           
volatile unsigned char AdcFlag = 0;

//�����ʶλ  1������   0�� û�е���
volatile unsigned char PowerDownFlag = 1;                   //Power down accidentally

//��������ʶλ  1���д���   0�� û�д���
volatile unsigned char PowerDownErrFlag = 0;                //Power down when PowerOn , Error BEEP!

//������ʶλ 1���ѿ���  0�� �ػ�������24V����
volatile unsigned char PowerOnFlag = 0;	

//���ȴ���Ĵ��������趨�Ĵ�����ʶλ   ���ȴ������
volatile unsigned char FanErrFlag = 0, FanErrCnt = 0;       //Fan speed exceed limits

//��Դ������������趨�Ĵ�����ʶλ����Դ��ѹ������Χ����
unsigned char BusVoltageErrFlag = 0, BusVoltageErrCnt = 0;  //Main 24V DC supply exceed limits

unsigned int FanSpeedMax = 150;    // normally, fan speed=3400 rpm,  2 pules/revolution, 113 pules each second
unsigned int FanSpeedMin = 60;

unsigned int BusVoltageMax = 160;  //  100 = 20 V,  180 = 28 v
unsigned int BusVoltageMin = 120;

//�ϴΰ���ֵ
unsigned char LastKey;
//���ΰ���ֵ
unsigned char CurretKey;
//��¼���µİ�������
unsigned char KeyCnt;
//���ּ��
unsigned char HoldCnt;

unsigned char AlarmCnt = 0;                         //���籨������
volatile unsigned char AlarmFlag = 0;               //���ڱ�����ʱ�򲻹رշ�����

unsigned short BusVoltage;                          //��Դ��ѹֵ
volatile unsigned char UartTransmit_Sec = 0;        //ÿ���ӷ������ݱ�ʶλ����ʱ�������λ��������������0��
unsigned char Current_State;                        //������ǰ״̬��0 ��������  1 ��������  2ϵͳ�Լ� 3���ض���  4׼��Ԥ��  5����Ԥ�� 6��������  7ȷ�Ͻ���   8���ƽ���
                                                    //              9 ϵͳά��  10����У׼  11���Ʋ�������   12 ϵͳ����1   13 ϵͳ����2
unsigned char  R_Selfcheck=1;                       //�Լ�����  0���Լ�û��ͨ�� 1 �� �Լ�ͨ��
 
unsigned short Fan_speed;                           //���������ٶ�                         
unsigned char UartTx_Buf[8];                        //��Ƭ�����������͵����ݱ��������
unsigned char UartReceiveBuf[9];                    //��Ƭ�����������������������ݱ��������
unsigned char EndReceive=0;                         //������һ�����ݴ�ִ�лظ��ı�־��                  
unsigned char Hard_VER=11,Soft_VER=11;              //Ӳ���汾������汾
unsigned int  TotalTimeAPU=0;                       //�ϵ������ʱ��
 //�Լ���
unsigned short SelfCheckErr=0xffff;  
//�����Լ�
unsigned char StartSelfTest=0;
 
 //�����ʶ��  //unsigned long DCB_ERR1
 //BIT0  ��Դ����
 //BIT1  ��Դ��ѹ����Ҫ��ķ�Χ��((BusVoltage>=22)&&(BusVoltage<=26))
 //BIT2  ����ת�ٲ���Ҫ��ķ�Χ��((Fan_speed=60)&&(Fan_speed<=150))
 unsigned char DCB_ERR1[8]={0,0,0,0,0,0,0,0};
 
/**

 */
int main(void)
{
    SystemCoreClockUpdate();        //ϵͳʱ�ӳ�ʼ��
//	Board_Init();           	
    uart_init();                    //���ڳ�ʼ�� 8N1
    
    timer32_1_init();               //timer32_1��ʼ����ÿ50Ms���	
    timer32_0_init();               //timer32_0��ʼ����ÿ1000Ms���
    
    CounterInit();                  //timer16��ʼ������������������ȡ����ת��   
    Ioout_init();                   //��ʼ����Ҫ��Դ���lpc11c14���Ƶ��豸���ţ�
    ADC_init();                     //��ʼ��AD�����ڶ�ȡ�����ⲿ24V�Ƿ�����Ҫ��ķ�Χ��
    exint0_init();                  //�ⲿ�жϳ�ʼ�������ڲ����ⲿ24V��û�е��磻       
    key_init();                     //������ʼ�����������ڿ����ػ�
    WWDT_init();
    while (1) 
    {
        Button_Detect();            //��ⰴ��
        CheckBusVolt();             //��ȡ��ѹADֵ
        Uart_Sent_Sec();            //ÿ���ӷ���һ��A0��A1;                     
        uart_cmd();                 //��Ӧ��λ��ָ��

        //����״̬�£����籨��2min��
        if ( PowerDownErrFlag && (0x06 == Current_State))                                                         
        {            
            for (AlarmCnt = 0; ((AlarmCnt < 90) && ( PowerDownErrFlag)); AlarmCnt++)
            {
                Chip_WWDT_Start(LPC_WWDT);
                AlarmBeep(2);
            }
            myDelay(3000);
        }
        
        if ( StartSelfTest==1)
        {
           StartSelfTest=0;
           SelfCheckErr=HardwareSelfTest();
        }
    }
}

// end of the file
