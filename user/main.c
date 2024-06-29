#include "stm32f10x.h"
#include "stm32f10x_pal.h"
//按键切换灯泡闪烁快慢：0-慢闪、1-正常速度、2-快闪
/*
使用裸机多任务编程结构，按键监测和LED闪烁各一个任务Proc
只能使用标准库函数编程

按键1:  PE4	按下低电平
LED3：	PB5 低电平亮 ——推挽模式

问题：EXTI机制的按键检测是否需要防抖机制？


*/

#define LED_N0_BLOCK_PROC

uint32_t press_times;
uint32_t LED_blink_speed;
uint32_t blinkSpeedSet[] = {100,500,1000};		//led blink interval(ms)
uint8_t key_event, led_event;


static void periph_clock_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB1ENR_TIM2EN, ENABLE);			
}

static void debug_uart_init(void)
{
	USART_InitTypeDef uart_debug;
	uart_debug.USART_BaudRate = 115200;
	uart_debug.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_debug.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	uart_debug.USART_Parity = USART_Parity_No;
	uart_debug.USART_StopBits = USART_StopBits_1;
	uart_debug.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &uart_debug);
	
	USART_Cmd(USART1, ENABLE);
}

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
	PAL_Delay(LED_blink_speed);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	PAL_Delay(LED_blink_speed);
#else
	static 	uint64_t led_event_ticks;		//下一次切换状态的时机ticks（电平保持期间，led_event_ticks略大于systick）
	if (led_event_ticks <= PAL_GetTick())	//一旦systick >= led_event_ticks时，表示延迟已结束
	{
		led_event_ticks = PAL_GetTick() + LED_blink_speed;		//再向后推迟一段时间（更新led_event_ticks）
		LED_trigger();
	}
#endif
}

static void key_detect_proc(void)
{
	uint8_t key_level;
	
	if (key_event == RESET)
		return;
	
	PAL_Delay(5);		//按键消抖
	key_level = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
	if (key_level == RESET)
	{
		press_times++;
		LED_blink_speed = blinkSpeedSet[press_times%(sizeof(blinkSpeedSet)/sizeof(uint32_t))];
	}
	else
	{
		;	//松手动作逻辑
	}
	key_event = RESET;
	
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

	while(1)
	{
		led_blink_proc();
		key_detect_proc();
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
