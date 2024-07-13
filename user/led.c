#include "stm32f10x.h"
#include "string.h"
#include "led.h"
#include "stm32f10x_pal.h"


#define LED_N0_BLOCK_PROC

uint32_t LED_blink_speed;
uint32_t blinkSpeedSet[] = {100,500,1000};		//led blink interval(ms)

uint64_t led_event_ticks;		//下一次切换状态的时机ticks（电平保持期间，led_event_ticks略大于systick）

uint32_t LED_GetBlinkSpeed(void)
{
	return LED_blink_speed;
}

void LED_SetBlinkSpeed(uint32_t blinkMs)
{
	LED_blink_speed = blinkMs;
}

void LED_BlinkSuspend(void)
{
	memset(&led_event_ticks, 0xFF, sizeof(led_event_ticks));
}

void LED_BlinkResume(void)
{
	led_event_ticks = PAL_GetTick();
}

void LED_trigger(void)
{
	uint8_t led_level = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5);
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, !led_level);
}

void led_blink_proc(void)
{	
#ifndef LED_N0_BLOCK_PROC
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	PAL_Delay(LED_blink_speed);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	PAL_Delay(LED_blink_speed);
#else
	//static 	uint64_t led_event_ticks;		//下一次切换状态的时机ticks（电平保持期间，led_event_ticks略大于systick）
	if (led_event_ticks <= PAL_GetTick())	//一旦systick >= led_event_ticks时，表示延迟已结束
	{
		led_event_ticks = PAL_GetTick() + LED_blink_speed;		//再向后推迟一段时间（更新led_event_ticks）
		LED_trigger();
	}
#endif
}

void LED_SwitchBlinkSpeed(uint8_t index)
{
	LED_SetBlinkSpeed(blinkSpeedSet[index%(sizeof(blinkSpeedSet)/sizeof(uint32_t))]);
}

void LED_init(void)
{
	LED_blink_speed = blinkSpeedSet[0];		//默认闪烁速度
	
	GPIO_InitTypeDef led_pb5;
	led_pb5.GPIO_Mode = GPIO_Mode_Out_OD;
	led_pb5.GPIO_Pin = GPIO_Pin_5;
	led_pb5.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &led_pb5);	
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);	//默认关灯
}
