/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.h
  * @author  铁头山羊stm32工作组
  * @version V2.0.0
  * @date    2024年1月18日
  * @brief   mpu6050驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2024 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/


#ifndef _STM32F10x_PAL_MPU6050_H_
#define _STM32F10x_PAL_MPU6050_H_
#include "stm32f10x.h"
#include "stm32f10x_pal.h"
#include "stm32f10x_pal_i2c.h"

#define MPU6050_DLFP_260Hz_256Hz 0x00
#define MPU6050_DLFP_184Hz_188Hz 0x01
#define MPU6050_DLFP_94Hz_98Hz   0x02
#define MPU6050_DLFP_44Hz_42Hz   0x03
#define MPU6050_DLFP_21Hz_20Hz   0x04
#define MPU6050_DLFP_10Hz_10Hz   0x05
#define MPU6050_DLFP_5Hz_5Hz     0x06
#define MPU6050_DLFP_RESERVED    0x07

#define MPU6050_GYROSCALERANGE_250     0x00  // 250°/秒
#define MPU6050_GYROSCALERANGE_500     0x08  // 500°/秒
#define MPU6050_GYROSCALERANGE_1000    0x10  // 1050°/秒
#define MPU6050_GYROSCALERANGE_2000    0x18  // 2000°/秒
																		   			 
#define MPU6050_ACCELSCALERANGE_2g     0x00  // 2g
#define MPU6050_ACCELSCALERANGE_4g     0x08  // 4g
#define MPU6050_ACCELSCALERANGE_8g     0x10  // 8g
#define MPU6050_ACCELSCALERANGE_16g    0x18  // 16g

typedef struct
{
	float AccelBias[3];
	float GyroBias[3];
	float TempBias;
} PalMPU6050_BiasTypeDef;

typedef struct
{
	PalI2C_HandleTypeDef *I2Cx;  // MPU6050所使用的I2C接口
	FlagStatus SA0;              // 从机地址选择线状态 
	uint16_t SampleRate;         // 采样率，4~1000Hz
	uint16_t MPU6050_DLPF;       // 设置数字低通滤波器
	uint16_t MPU6050_AccelFsr;   // 加速度计量程
	uint16_t MPU6050_GyroFsr;    // 陀螺仪量程
	float Tau;                   // 互补滤波器参数Tau
	PalMPU6050_BiasTypeDef Bias;
}PalMPU6050_InitTypeDef;

typedef struct
{
	float gx; // 陀螺仪X轴数据
	float gy;
	float gz;
	float temp; // 温度
	float ax; // 加速度传感器X轴数据
	float ay;
	float az;
	float yaw;   // 偏航角
	float roll;  // 翻滚角
	float pitch; // 俯仰角
} PalMPU6050_DataTypeDef;

typedef struct
{
	PalMPU6050_InitTypeDef Init;
	float SampleInterval;          // 采样间隔，单位s
	uint32_t ProcRunInterval;      // 执行进程函数的间隔，单位ms
	uint64_t ProcNextExecTime;     // 下次执行进程函数的时间
	PalMPU6050_DataTypeDef Cache; // 缓存的数据
	float GyroScaleFactor;         // 缩放因子，从原始数据转换为实际读数时使用
	float AccelScaleFactor;
	float TempratureScaleFactor;
	float Alpha;                   // 阿尔法的值，用于传感器融合
	FlagStatus FusionComputed;     // 传感器融合是否已经计算过
}PalMPU6050_HandleTypeDef;

       void PAL_MPU6050_InitHandle(PalMPU6050_HandleTypeDef *Handle);
ErrorStatus PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle);
ErrorStatus PAL_MPU6050_Calibrate(PalMPU6050_HandleTypeDef *Handle);
ErrorStatus PAL_MPU6050_Proc(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetTemperature(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle);
      float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle);
     
#endif

