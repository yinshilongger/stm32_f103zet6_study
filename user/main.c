#include "stm32f10x.h"
#include "string.h"
#include "stm32f10x_pal.h"
#include "debug.h"
#include "communication.h"
#include "button.h"
#include "led.h"
#include "timer_driver.h"

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


static void periph_clock_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB1ENR_TIM2EN, ENABLE);			
}

void set_led_breath(void)
{
	float second = TIMER_GetTick() * 0.001;
	float duty = 0.5 + 0.5*sin(0.6*3.14*second);		//duty:between 0-1
	TIM_SetCompare2(TIM1, 999*duty);				//之所以不写1000而写999的原因是：虽然周期是1000，但是如果duty是1，那么装载CCR中的值就大于ARR了。
}

int main()
{
	PAL_Init();				//for systick
	periph_clock_init();
	debug_uart_init();
	APP_Timer_init();
	LED_init();
	Button_init();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//debug("init over");
	printf("Initualiize finished\n");
	while(1)
	{
		//debug print最好不要在任务中使用，因为会发送过程会使得任务执行时间过长
		led_blink_proc();			//控制LED闪烁的线程：闪烁加速、闪烁减速、暂停闪烁、恢复闪烁
		Button_detect_proc();
		uart_recv_detect_proc();
		Timer_OC_breath_led_proc();	//输出比较控制呼吸灯效果
	}
	
	return 0;
}

