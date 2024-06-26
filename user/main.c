#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "stm32f10x_pal_usart.h"
#include "math.h"
#include "stm32f10x_pal_mpu6050.h"

PalUSART_HandleTypeDef hUSART1;
static PalI2C_HandleTypeDef hI2C1;
static PalMPU6050_HandleTypeDef hMPU6050;

static void Print_Proc(void);

#define VER(__val__) if(__val__!=SUCCESS) while(1);

int main(void)
{
	PAL_Init();
	
	PAL_USART_InitHandle(&hUSART1);
	hUSART1.Init.USARTx = USART1;
	hUSART1.Init.BaudRate = 115200;
	PAL_USART_Init(&hUSART1);
	
	hI2C1.Init.I2Cx = I2C1;
	hI2C1.Init.I2C_ClockSpeed = 400000;
	hI2C1.Init.I2C_DutyCycle = I2C_DutyCycle_2;
	PAL_I2C_Init(&hI2C1);
	
	PAL_MPU6050_InitHandle(&hMPU6050);
	hMPU6050.Init.I2Cx = &hI2C1;
	
	hMPU6050.Init.Bias.AccelBias[0] = 0.049;
	hMPU6050.Init.Bias.AccelBias[1] = 0.005;
	hMPU6050.Init.Bias.AccelBias[2] = 0.082;
	hMPU6050.Init.Bias.GyroBias[0]  = -1.430;
	hMPU6050.Init.Bias.GyroBias[1]  = 0.592;
	hMPU6050.Init.Bias.GyroBias[2]  = 0.806;

// accel_bias=(0.049,0.005,1.082) gyro_bias=(-1.430,0.592,0.806)
	
	VER(PAL_MPU6050_Init(&hMPU6050))
	
	while(1)
	{
		PAL_MPU6050_Proc(&hMPU6050);
		Print_Proc();
	}
}

static void Print_Proc(void)
{
	PAL_PERIODIC_PROC(10);
	
	float ax    = PAL_MPU6050_GetAccelX(&hMPU6050);
	float ay    = PAL_MPU6050_GetAccelY(&hMPU6050);
	float az    = PAL_MPU6050_GetAccelZ(&hMPU6050);
	float gx    = PAL_MPU6050_GetGyroX(&hMPU6050);
	float gy    = PAL_MPU6050_GetGyroY(&hMPU6050);
	float gz    = PAL_MPU6050_GetGyroZ(&hMPU6050);
	float yaw   = PAL_MPU6050_GetYaw(&hMPU6050);
	float roll  = PAL_MPU6050_GetRoll(&hMPU6050);
	float pitch = PAL_MPU6050_GetPitch(&hMPU6050);
	float temp  = PAL_MPU6050_GetTemperature(&hMPU6050);
	
	PAL_USART_Printf(&hUSART1, "$%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f;\r\n", 
	ax, ay, az, 
	gx, gy, gz, 
	yaw, roll, pitch, temp);
}

void USART1_IRQHandler(void)
{
	PAL_USART_IRQHandler(&hUSART1);
}
