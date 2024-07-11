#include "stm32f10x.h"
#include "string.h"
#include "stm32f10x_pal.h"
#include "timer.h"
//按键切换灯泡闪烁快慢：0-慢闪、1-正常速度、2-快闪
/*
使用裸机多任务编程结构，按键监测和LED闪烁各一个任务Proc
只能使用标准库函数编程

按键1:  PE4	按下低电平
LED3：	PB5 低电平亮 ——推挽模式

问题：
1. EXTI机制的按键检测是否需要防抖机制？存在EXTI模式下按键抖动造成多次进入中断。不能中断就近处理，需要在任务中消抖。
2. 串口发送debug，可以使用while循环检测TXE标记发送，也可以按字节使用串口中断发送，需要提前将发送数据放入队列，注意控制TXE中断屏蔽与使能。
若使用printf来向串口发送调试信息，则需要禁止半主机模式，重写putc接口，并取消microLib微库的使用。

*/

#define LED_N0_BLOCK_PROC
#define DEBUG		//usart2 printf, no use microLib

typedef struct UART_RecvInfo
{
	uint8_t buf[32];		//usart recv data max 32 Byte every times
	uint8_t len;				//任务读取后要清零
	uint8_t complete;			//0-recv info incomplete 1-recv complete(with \r\n) 中断中SET，任务中读取后RESET
}UART_recv_t;


uint32_t press_times;
uint32_t LED_blink_speed;
uint32_t blinkSpeedSet[] = {100,500,1000};		//led blink interval(ms)
uint8_t key_event, led_event;
uint64_t led_event_ticks;		//下一次切换状态的时机ticks（电平保持期间，led_event_ticks略大于systick）
UART_recv_t uart_recv_info;

static void periph_clock_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB1ENR_TIM2EN, ENABLE);			
}

static void debug_uart_init(void)
{
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

static void LED_init(void)
{
	LED_blink_speed = blinkSpeedSet[0];		//默认闪烁速度
	
	GPIO_InitTypeDef led_pb5;
	led_pb5.GPIO_Mode = GPIO_Mode_Out_OD;
	led_pb5.GPIO_Pin = GPIO_Pin_5;
	led_pb5.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &led_pb5);	
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);	//默认关灯
}

static void key_init(void)
{
	press_times = 0;
	key_event = 0;
	
	GPIO_InitTypeDef key_pe4;
	key_pe4.GPIO_Mode = GPIO_Mode_IPU;
	key_pe4.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE, &key_pe4);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
}

static void key_exti_nvic_config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static void exti_init(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;		//Mask Regisster
	EXTI_Init(&EXTI_InitStruct);
}

static void LED_trigger(void)
{
	uint8_t led_level = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5);
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, !led_level);
}

static void led_blink_proc(void)
{	
#ifndef LED_N0_BLOCK_PROC
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	TIMER_Delay(LED_blink_speed);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	TIMER_Delay(LED_blink_speed);
#else
	//static 	uint64_t led_event_ticks;		//下一次切换状态的时机ticks（电平保持期间，led_event_ticks略大于systick）
	if (led_event_ticks <= TIMER_GetTick())	//一旦systick >= led_event_ticks时，表示延迟已结束
	{
		led_event_ticks = TIMER_GetTick() + LED_blink_speed;		//再向后推迟一段时间（更新led_event_ticks）
		LED_trigger();
	}
#endif
}

static void key_detect_proc(void)
{
	uint8_t key_level;
	
	if (key_event == RESET)
		return;
	
	TIMER_Delay(5);		//按键消抖
	key_level = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
	if (key_level == RESET)
	{
		press_times++;
		LED_blink_speed = blinkSpeedSet[press_times%(sizeof(blinkSpeedSet)/sizeof(uint32_t))];
	}
	else
	{
		;	//松手动作逻辑(若使用该逻辑，需要同时修改EXTI的触发为松手电平边沿类型)
	}
	key_event = RESET;
	
}


static void uart_recv_clear(void)
{
	//memset(&uart_recv_info, 0, sizeof(uart_recv_len));	//the same effect as below
	uart_recv_info.len = 0;
	uart_recv_info.complete = 0;
}


static void uart_recv_detect(void)
{
	if (uart_recv_info.complete != 0 && uart_recv_info.len != 0)
	{
		if (uart_recv_info.buf[0] == 'a' && uart_recv_info.len == 1)
		{
			memset(&led_event_ticks, 0xFF, sizeof(led_event_ticks));
		}
		else if (uart_recv_info.buf[0] == 'b' && uart_recv_info.len == 1)
		{
			led_event_ticks = PAL_GetTick();
		}
		uart_recv_clear();
	}	
}

void set_led_breath(void)
{
	float second = TIMER_GetTick() * 0.001;
	float duty = 0.5 + 0.5*sin(0.6*3.14*second);		//duty:between 0-1
	TIM_SetCompare2(TIM1, 999*duty);				//之所以不写1000而写999的原因是：虽然周期是1000，但是如果duty是1，那么装载CCR中的值就大于ARR了。
}

int main()
{
	PAL_Init();
	periph_clock_init();
	debug_uart_init();
	LED_init();
	key_init();
	exti_init();
	key_exti_nvic_config();		//key nvic config
	APP_Timer_init();			//Period:1ms, prescaler:1us
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	memset(&uart_recv_info, 0, sizeof(uart_recv_info));
	//debug("init over");
	printf("Initualiize finished\n");
	while(1)
	{
		//debug print最好不要在任务中使用，因为会发送过程会使得任务执行时间过长
		led_blink_proc();		
		key_detect_proc();
		uart_recv_detect();
		set_led_breath();

	}
	
	return 0;
}

void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4);
		key_event = SET;		//向按键Proc发送事件，在按键Proc中去做按键防抖5ms
	}
}

void USART2_IRQHandler(void)
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


