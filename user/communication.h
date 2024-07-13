#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "stm32f10x.h"

#define DEBUG		//usart2 printf, no use microLib

typedef struct UART_RecvInfo
{
	uint8_t buf[32];		//usart recv data max 32 Byte every times
	uint8_t len;				//任务读取后要清零
	uint8_t complete;			//0-recv info incomplete 1-recv complete(with \r\n) 中断中SET，任务中读取后RESET
}UART_recv_t;


void debug_uart_init(void);
void uart_recv_detect_proc(void);

#endif


