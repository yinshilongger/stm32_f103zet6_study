#include "string.h"
#include "communication.h"
#include "led.h"

static UART_recv_t uart_recv_info;


static void uart_recv_clear(void)
{
	//memset(&uart_recv_info, 0, sizeof(uart_recv_len));	//the same effect as below
	uart_recv_info.len = 0;
	uart_recv_info.complete = 0;
}


void uart_recv_detect_proc(void)
{
	if (uart_recv_info.complete != 0 && uart_recv_info.len != 0)
	{
		if (uart_recv_info.buf[0] == 'a' && uart_recv_info.len == 1)
		{
			LED_BlinkSuspend();
		}
		else if (uart_recv_info.buf[0] == 'b' && uart_recv_info.len == 1)
		{
			LED_BlinkResume();
		}
		uart_recv_clear();
	}	
}

void debug_uart_init(void)
{
	memset(&uart_recv_info, 0, sizeof(uart_recv_info));

	//init gpio for usart2 tx-PA2 and rx-PA3
	GPIO_InitTypeDef gpio_initStruct;
	gpio_initStruct.GPIO_Pin = GPIO_Pin_2;
	gpio_initStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_initStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpio_initStruct);
	gpio_initStruct.GPIO_Pin = GPIO_Pin_3;
	gpio_initStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &gpio_initStruct);
	
	//init usart2 paramter
	USART_InitTypeDef uart_debug;
	uart_debug.USART_BaudRate = 115200;
	uart_debug.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_debug.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	uart_debug.USART_Parity = USART_Parity_No;
	uart_debug.USART_StopBits = USART_StopBits_1;
	uart_debug.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &uart_debug);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);		//注意：之前遗漏这一条配置项了
	USART_Cmd(USART2, ENABLE);
	
	NVIC_InitTypeDef nvic_initStruct;
	nvic_initStruct.NVIC_IRQChannel = USART2_IRQn;
	nvic_initStruct.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_initStruct.NVIC_IRQChannelSubPriority = 0;
	nvic_initStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_initStruct);
}


void COMUNICATION_RECV_IRQHandler(void)
{
	uint8_t recv;
	//串口接收中断（完整数据必须以\r\n结尾）
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		//USART_ClearITPendingBit(USART2, USART_IT_RXNE);		//可省略，先读SR，再读RDR即可清除
		recv = USART_ReceiveData(USART2);
		uart_recv_info.buf[uart_recv_info.len] = recv;
		uart_recv_info.len++;
		if (uart_recv_info.len > sizeof(uart_recv_info.buf))	//接收数据长度越界
		{
			//数据接收出错
			uart_recv_info.len = 0;
			return;
		}
		
		if (uart_recv_info.buf[uart_recv_info.len - 2] == '\r'		\
			&& uart_recv_info.buf[uart_recv_info.len - 1] == '\n')
		{
			uart_recv_info.complete = 1;
			uart_recv_info.buf[uart_recv_info.len - 2] = '\0';		//添加字符串结束符
			uart_recv_info.len -= 2;
		}
	}
}
