#include "stm32f10x.h"
#include "debug.h"

#ifdef DEBUG
#include "stdio.h"		//for printf

//以下代码,支持printf函数,而不需要选择use MicroLIB	  
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
	USART2->DR = (u8) ch;      
	return ch;
}
#else

//参数：待打印的字符数组（\0结尾），此接口会占用较长时间发送数据，若数据不以\0结束则无法退出，谨慎使用。
static void debug(uint8_t *str)
{
	uint8_t i = 0;
	//while (str[i] != '\0' && USART_GetFlagStatus(USART2, USART_FLAG_TXE))		//不能这样写，一旦TXE没有准备好就退出循环，停止后续发送了
	while (str[i] != '\0')
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET){}		//TXE标志只能单独在while中循环，若加入其他判断则会有影响
		USART_SendData(USART2, str[i]);
		i++;
	}
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
}

#endif

