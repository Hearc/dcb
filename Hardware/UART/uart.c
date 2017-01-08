/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          uart.c
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

#include "uart.h"
#include "ioout.h"

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];
extern volatile unsigned char UartTransmit_Sec; 
extern unsigned char UartTx_Buf[8];
extern unsigned char UartReceiveBuf[9];
extern unsigned char Current_State;
extern unsigned char Hard_VER,Soft_VER;   
extern unsigned int FanSpeedMax;   
extern unsigned int FanSpeedMin;

extern unsigned int BusVoltageMax;  
extern unsigned int BusVoltageMin;
extern unsigned char BusVoltage;  
extern unsigned char DCB_ERR1[8]; 
extern volatile unsigned char PowerOnFlag;
extern unsigned int TotalTimeAPU;
extern unsigned short Fan_speed;
extern unsigned char R_Selfcheck;
extern volatile unsigned char FanErrFlag;
extern unsigned char BusVoltageErrFlag;
extern unsigned char StartSelfTest;
extern unsigned short SelfCheckErr;

/*****************************************************************************
 *  PIO1_6 used for RXD ，PIO0_7 used for TXD
 ****************************************************************************/
static void Init_UART_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_FUNC1 | IOCON_MODE_INACT);     /* PIO0_18 used for RXD */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, IOCON_FUNC1 | IOCON_MODE_INACT);     /* PIO0_19 used for TXD */
#elif (defined(BOARD_NXP_XPRESSO_11C24) || defined(BOARD_MCORE48_1125))
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));
#else
#error "No Pin muxing defined for UART operation"
#endif
}

/*****************************************************************************
 * 处理串口中断
 ****************************************************************************/
void UART_IRQHandler(void)
{
    /* Want to handle any errors? Do it here. */

    /* Use default ring buffer handler. Override this with your own
       code if you need more capability. */
    Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

/*****************************************************************************
 * 串口初始化
 * Baud 115200;
 * 8bit,1stop,No PARITY
 ****************************************************************************/
void uart_init()
{	
    Init_UART_PinMux();

    /* Setup UART for 115.2K8N1 */
    Chip_UART_Init(LPC_USART);
    Chip_UART_SetBaud(LPC_USART, 115200);
    Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
    Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
    Chip_UART_TXEnable(LPC_USART);

    /* Before using the ring buffers, initialize them using the ring
         buffer init function */
    RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
    RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

    /* Enable receive data and line status interrupt */
    Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART0_IRQn, 1);
    NVIC_EnableIRQ(UART0_IRQn); 
}

/*****************************************************************************
 * 每秒钟向主机发送电压值和风扇转速和错误位；
 ****************************************************************************/
void Uart_Sent_Sec(void)
{
    unsigned char i;
    if((UartTransmit_Sec==1)&&(PowerOnFlag==1))
    {
        UartTransmit_Sec=0;
        
        UartTx_Buf[0]=0XA0;
        UartTx_Buf[1]=Current_State;
        UartTx_Buf[2]=(SelfCheckErr>>8)&0xff;
        UartTx_Buf[3]=SelfCheckErr&0xff;
        UartTx_Buf[4]=DCB_ERR1[4];
        UartTx_Buf[5]=DCB_ERR1[5];
        UartTx_Buf[6]=DCB_ERR1[6];
        UartTx_Buf[7]=DCB_ERR1[7];
        for(i=0;i<8;i++)
        {
            Chip_UART_SendByte(LPC_USART,UartTx_Buf[i]);    
        }

        UartTx_Buf[0]=0XA1;
        UartTx_Buf[1]=Current_State;
        UartTx_Buf[2]=0x00;
        UartTx_Buf[3]=0x00;
        UartTx_Buf[4]=0x00;
        UartTx_Buf[5]=BusVoltage;
        UartTx_Buf[6]=(Fan_speed>>8)&0xff;
        UartTx_Buf[7]=Fan_speed&0xff;
        for(i=0;i<8;i++)
        {
            Chip_UART_SendByte(LPC_USART,UartTx_Buf[i]);    
        }
    }
}

/*****************************************************************************
 * 响应上位机指令
 * 指令格式参考电源板技术方案
 ****************************************************************************/
void uart_cmd(void)
{
    uint8_t bytes = 0;
    unsigned char i;
    
    bytes = Chip_UART_ReadRB(LPC_USART, &rxring, UartReceiveBuf, 8);
    if (bytes > 7)
    {
        EndReceive=0;
        UartTx_Buf[0]=UartReceiveBuf[0]+0x60;
        UartTx_Buf[1]=UartReceiveBuf[1];
        Current_State=UartReceiveBuf[1];
        switch(UartReceiveBuf[0])
        {
            case  0x50:                                                       //发送软，硬件版本和总上电时间
            UartTx_Buf[2] = Hard_VER;
            UartTx_Buf[3] = Soft_VER;
            UartTx_Buf[4] = (TotalTimeAPU>>16)&0xff;
            UartTx_Buf[5] = (TotalTimeAPU>>8)&0xff;
            UartTx_Buf[6] = (TotalTimeAPU)&0xff;
            if((!FanErrFlag)&&(!BusVoltageErrFlag))
            {
            R_Selfcheck=1;
            }
            UartTx_Buf[7] = R_Selfcheck;
            for(i=0;i<8;i++)
            {
                Chip_UART_SendByte(LPC_USART,UartTx_Buf[i]);    
            }
            break;
            
            case  0x51:                                                        //发送风扇速度上，下限，发送电源电压上下限值
            UartTx_Buf[2] = (FanSpeedMax>>8)&0xff;
            UartTx_Buf[3] = (FanSpeedMax)&0xff;
            UartTx_Buf[4] = (FanSpeedMin>>8)&0xff;
            UartTx_Buf[5] = (FanSpeedMin)&0xff;
            UartTx_Buf[6] = (BusVoltageMax)&0xff;
            UartTx_Buf[7] = (BusVoltageMin)&0xff;
            for(i=0;i<8;i++)
            {
                Chip_UART_SendByte(LPC_USART,UartTx_Buf[i]);    
            }
            break;
            
            case  0x60:                                                         //主机状态切换
            if(UartReceiveBuf[1]==0x02)                                         //自检  界面        
            {
                if(UartReceiveBuf[7]==0x01)                                     //执行自检
                {
                    StartSelfTest=1;
                    SelfCheckErr=0x0001;
                }
            }
            break;
             
            case  0x61:                                                         // 写入风扇速度大小，电源电压上下限
            FanSpeedMax=(UartReceiveBuf[2]<<8)|UartReceiveBuf[3];
            FanSpeedMin=(UartReceiveBuf[4]<<8)|UartReceiveBuf[5];
            BusVoltageMax=UartReceiveBuf[6];
            BusVoltageMin=UartReceiveBuf[7];
            break;
			 			 
            default :                                                           //
            Chip_UART_Send(LPC_USART,"erro",4);
            break;	 				 				 
        }
    } //end of if
}

//end of the file

