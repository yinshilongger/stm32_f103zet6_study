#include "timer.h"

static volatile uint64_t gTicks;

void APP_Timer_init(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1EN, ENABLE);
	
	//打开ARR的影子寄存器特性
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	
	//初始化时基单元
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Prescaler = 71;		//1us
	TIM_InitStruct.TIM_Period = 999;		//1ms
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	//用于输入捕获？
	TIM_TimeBaseInit(TIM1, &TIM_InitStruct);
	
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);	//使影子寄存器值生效，清空CNT计数
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
}

uint64_t TIMER_GetTick(void)
{
	return gTicks;
}

uint64_t TIMER_GetTickUs(void)
{
	uint64_t curTickUs = TIM_GetCounter(TIM1);		//CNT计数器的值单位为当前定时器配置的分辨率ms
	return gTicks * 1000 + curTickUs;
}

void TIMER_Delay(uint16_t delayMs)
{
	uint64_t destTicks = gTicks + delayMs;
	while(destTicks > gTicks){}
}

void TIME_DelayUs(uint16_t delayUs)
{
#if 1
	uint64_t destTickUs = TIMER_GetTickUs() + delayUs;
	while(TIMER_GetTickUs() < destTickUs){}	
#else
	//错误写法
	uint64_t curTickUs = TIMER_GetTickUs();
	uint64_t destTickUs = curTickUs + delayUs;
	while(curTickUs < destTicksUs){}		//永不退出
#endif
}

void TIM1_UP_IRQHandler(void)
{
	//TIM1的Update中断并非全局中断，因此无需判断中断源
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	gTicks++;
}

