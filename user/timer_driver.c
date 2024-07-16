#include "timer_driver.h"
#include "stdio.h"
#include "math.h"
static volatile uint64_t gTicks;

static void Timer_TimeBaseInit(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	//打开ARR的影子寄存器特性
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	
	//初始化时基单元
	TIM_TimeBaseInitTypeDef TIM_InitStruct = {0};
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Prescaler = 71;		//1us
	TIM_InitStruct.TIM_Period = 999;		//1ms
	//TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	//用于输入滤波和死区时间，时基单元用不到，后续可通过TIM_SetClockDivision配置
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_InitStruct);
	
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);	//使影子寄存器值生效，清空CNT计数
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

}

static void Timer_ICInit(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	//初始化输入捕获通道1（PA8）
	TIM_ICInitTypeDef IC_InitStruct = {0};
	IC_InitStruct.TIM_Channel = TIM_Channel_1;
	IC_InitStruct.TIM_ICFilter = 0x08;								//filter 1.3us
	IC_InitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;			//fall EDGE detect to capture
	IC_InitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;					//every 1 ARR counts generate IC event
	IC_InitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;		//channel 1 instand of channel 2
	TIM_ICInit(TIM1, &IC_InitStruct);
	
	TIM_SelectHallSensor(TIM1, DISABLE);
	TIM_SetClockDivision(TIM1, TIM_CKD_DIV2);						//配置捕获通道的滤波时钟分频，配合输入捕获TIM_ICFilter一起计算滤波宽度
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//没有配置TIM1通道1的重映射，尚不清楚PA8默认usart1的CLK还是TIM1Channel1？
	//GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE);		
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);				//通道1的输入捕获中断使能
	
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);				//使能通道1
}

static void Timer_OCInit(void)
{
	//TIM1的通道2(PA9) 输出比较 初始化（因为使用同一个TIM1，所以周期一致且不可更改）
	TIM_OCInitTypeDef OC_InitStruct = {0};
	OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM1;		//PWM输出模式1
	OC_InitStruct.TIM_Pulse = 0;					//CCR初始值，因为只有在init之后调用TIM_OC_SetCompare才能设定CCR，在这之前Init之后的的CCR初值表明了在这时间段内OC的输出状态
	OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;			//正极性
	OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;		//只闭合本通道的使能开关
	TIM_OC2Init(TIM1, &OC_InitStruct);							//通道2初始化
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;		//GPIO的输出比较速率应当参考PSC预分频的配置（因为输出比较的切换频率取决于CNT的变化速率，而CNT的变化速率取决于PSC的配置），当前PSC=71（最小1us变化一次，也即1MHz）72MHz/72 = 1Mhz
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//没有配置TIM1通道2的重映射，尚不清楚PA9默认usart1的TX还是TIM1Channel2？
	//GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE);		

//	不清楚是否有输出比较中断？
//	NVIC_InitStruct.NVIC_IRQChannel = TIM1_CC_IRQn;
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStruct);
	
//	TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE);
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);						//闭合输出比较通道总开关MOE
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);		//更新CCR影子寄存器的配置参数
	TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);		//使能通道2
}

void APP_Timer_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1EN, ENABLE);
	Timer_TimeBaseInit();
	Timer_ICInit();
	Timer_OCInit();
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

void TIMER_DelayUs(uint16_t delayUs)
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

void Timer_OC_breath_led_proc(void)
{
	float second = TIMER_GetTick() * 0.001;
	float duty = 0.5 + 0.5*sin(0.6*3.14*second);		//duty:between 0-1
	TIM_SetCompare2(TIM1, 999*duty);				//之所以不写1000而写999的原因是：虽然周期是1000，但是如果duty是1，那么装载CCR中的值就大于ARR了。
}


void APP_TIM1_UP_IRQHandler(void)
{
	//TIM1的Update中断并非全局中断，因此无需判断中断源
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		gTicks++;
	}
}

void APP_TIM1_CC_IRQHandler(void)
{
	uint16_t capture;
//	static uint64_t last_ticks;
//	static uint16_t last_cnt;
	
	//TIM1 通道1，用于输入捕获通道2输出的PWM波的CCR，CCR在呼吸灯进程中被动态修改。IC1捕获的CC值与OC2配置的CC值相等，因为两通道有共同的时基单元。
	if (TIM_GetITStatus(TIM1,TIM_IT_CC1) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		capture = TIM_GetCapture1(TIM1);		//单位：预分频配置的精度（1us）
		
		//时间差（us） = （当前微秒 - 上次微秒）+ （当前毫秒 - 上次毫秒）* 1000
		//difference = (capture - last_cnt) + (now_tick - last_ticks) * 1000;
		
		printf("CCR:%d\r\n", capture);
	}
}
