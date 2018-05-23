
/**
  ******************************************************************************
  * @file    /uartx.h 
  * @author  MMY
  * @version V0.0.1
  * @date    2017-4-1
  * @brief   a package of uartx
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2017 CQT Quartz. Co., Ltd.
  *
  ******************************************************************************
  */




#ifndef  _UART_X_
#define  _UART_X_


#include "cdv_include.h" 



void USART_Configuration(void);

/*�����������*/

void USARTSend(u8 *buf ,u16 len ,u8 no);
void USARTTR(u8 *txbuf,u16 txlen ,u8* rxbuf ,u8 rxbufLen,u8* rxlen ,u8 no);
void USARTx_IRQHandler(USART_TypeDef* USARTx);
void USARTSet(u32 bound, u16 wordLength, u16 stopBits, u16 parity, u8 no);

#endif

