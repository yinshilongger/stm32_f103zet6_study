#ifndef __TIMER_DRIVER_H__
#define __TIMER_DRIVER_H__
#include "stm32f10x.h"
#include "stm32f10x_tim.h"

void APP_Timer_init(void);
uint64_t TIMER_GetTick(void);
uint64_t TIMER_GetTickUs(void);
void TIMER_Delay(uint16_t delayMs);
void TIMER_DelayUs(uint16_t delayUs);
void Timer_OC_breath_led_proc(void);
void APP_TIM1_UP_IRQHandler(void);
void APP_TIM1_CC_IRQHandler(void);
#endif
