#ifndef __LED_H__
#define __LED_H__

#include "stm32f10x.h"

void LED_init(void);
void led_blink_proc(void);
uint32_t LED_GetBlinkSpeed(void);
void LED_SetBlinkSpeed(uint32_t binlkMs);
void LED_SwitchBlinkSpeed(uint8_t index);
void LED_BlinkSuspend(void);
void LED_BlinkResume(void);

#endif
