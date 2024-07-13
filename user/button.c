#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "button.h"
#include "led.h"


static uint32_t press_times;
static uint8_t Button_event;

static void exti_init(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;		//Mask Regisster
	EXTI_Init(&EXTI_InitStruct);
}


static void exti_nvic_config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void Button_init(void)
{
	press_times = 0;
	Button_event = 0;
	
	GPIO_InitTypeDef Button_pe4;
	Button_pe4.GPIO_Mode = GPIO_Mode_IPU;
	Button_pe4.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE, &Button_pe4);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
	
	exti_init();
	exti_nvic_config();		//button nvic config

}

void Button_detect_proc(void)
{
	uint8_t Button_level;
	
	if (Button_event == RESET)
		return;
	
	PAL_Delay(5);		//按键消抖
	Button_level = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
	if (Button_level == RESET)
	{
		press_times++;
		LED_SwitchBlinkSpeed(press_times);
	}
	else
	{
		;	//松手动作逻辑(若使用该逻辑，需要同时修改EXTI的触发为松手电平边沿类型)
	}
	Button_event = RESET;
	
}
void Button_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4);
		Button_event = SET;		//向按键Proc发送事件，在按键Proc中去做按键防抖5ms
	}
}

