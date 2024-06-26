/**
  ******************************************************************************
  * @file    stm32f10x_pal_mpu6050.c
  * @author  铁头山羊stm32工作组
  * @version V1.0.0
  * @date    2023年2月24日
  * @brief   mpu6050驱动
  ******************************************************************************
  * @attention
  * Copyright (c) 2022 - 2023 东莞市明玉科技有限公司. 保留所有权力.
  ******************************************************************************
*/

#include "stm32f10x_pal_mpu6050.h"
#include "stm32f10x_pal.h"

/* mpu6050寄存器地址列表 */
#define MPU6050_REG_SELF_TEST_X        0x0d
#define MPU6050_REG_SELF_TEST_Y        0x0e
#define MPU6050_REG_SELF_TEST_Z        0x0f
#define MPU6050_REG_SELF_TEST_A        0x10
#define MPU6050_REG_SMPRT_DIV          0x19
#define MPU6050_REG_CONFIG             0x1a
#define MPU6050_REG_GYRO_CONFIG        0x1b
#define MPU6050_REG_ACCEL_CONFIG       0x1c
#define MPU6050_REG_FIFO_EN            0x23
#define MPU6050_REG_I2C_MST_CTRL       0x24
#define MPU6050_REG_I2C_SLV0_ADDR      0x25
#define MPU6050_REG_I2C_SLV0_REG       0x26
#define MPU6050_REG_I2C_SLV0_CTRL      0x27
#define MPU6050_REG_I2C_SLV1_ADDR      0x28
#define MPU6050_REG_I2C_SLV1_REG       0x29
#define MPU6050_REG_I2C_SLV1_CTRL      0x2a
#define MPU6050_REG_I2C_SLV2_ADDR      0x2b
#define MPU6050_REG_I2C_SLV2_REG       0x2c
#define MPU6050_REG_I2C_SLV2_CTRL      0x2d
#define MPU6050_REG_I2C_SLV3_ADDR      0x2e
#define MPU6050_REG_I2C_SLV3_REG       0x2f
#define MPU6050_REG_I2C_SLV3_CTRL      0x30
#define MPU6050_REG_I2C_SLV4_ADDR      0x31
#define MPU6050_REG_I2C_SLV4_REG       0x32
#define MPU6050_REG_I2C_SLV4_DO        0x33
#define MPU6050_REG_I2C_SLV4_CTRL      0x34
#define MPU6050_REG_I2C_SLV4_DI        0x35
#define MPU6050_REG_I2C_MST_STATUS     0x36
#define MPU6050_REG_INT_PIN_CFG        0x37
#define MPU6050_REG_INT_ENABLE         0x38
#define MPU6050_REG_INT_STATUS         0x3a
#define MPU6050_REG_ACCEL_XOUT_H       0x3b
#define MPU6050_REG_ACCEL_XOUT_L       0x3c
#define MPU6050_REG_ACCEL_YOUT_H       0x3d
#define MPU6050_REG_ACCEL_YOUT_L       0x3e
#define MPU6050_REG_ACCEL_ZOUT_H       0x3f
#define MPU6050_REG_ACCEL_ZOUT_L       0x40
#define MPU6050_REG_TEMP_OUT_H         0x41
#define MPU6050_REG_TEMP_OUT_L         0x42
#define MPU6050_REG_GYRO_XOUT_H        0x43
#define MPU6050_REG_GYRO_XOUT_L        0x44
#define MPU6050_REG_GYRO_YOUT_H        0x45
#define MPU6050_REG_GYRO_YOUT_L        0x46
#define MPU6050_REG_GYRO_ZOUT_H        0x47
#define MPU6050_REG_GYRO_ZOUT_L        0x48
#define MPU6050_REG_EXT_SENS_DATA_00   0x49
#define MPU6050_REG_EXT_SENS_DATA_01   0x4a
#define MPU6050_REG_EXT_SENS_DATA_02   0x4b
#define MPU6050_REG_EXT_SENS_DATA_03   0x4c
#define MPU6050_REG_EXT_SENS_DATA_04   0x4d
#define MPU6050_REG_EXT_SENS_DATA_05   0x4e
#define MPU6050_REG_EXT_SENS_DATA_06   0x4f
#define MPU6050_REG_EXT_SENS_DATA_07   0x50
#define MPU6050_REG_EXT_SENS_DATA_08   0x51
#define MPU6050_REG_EXT_SENS_DATA_09   0x52
#define MPU6050_REG_EXT_SENS_DATA_10   0x53
#define MPU6050_REG_EXT_SENS_DATA_11   0x54
#define MPU6050_REG_EXT_SENS_DATA_12   0x55
#define MPU6050_REG_EXT_SENS_DATA_13   0x56
#define MPU6050_REG_EXT_SENS_DATA_14   0x57
#define MPU6050_REG_EXT_SENS_DATA_15   0x58
#define MPU6050_REG_EXT_SENS_DATA_16   0x59
#define MPU6050_REG_EXT_SENS_DATA_17   0x5a
#define MPU6050_REG_EXT_SENS_DATA_18   0x5b
#define MPU6050_REG_EXT_SENS_DATA_19   0x5c
#define MPU6050_REG_EXT_SENS_DATA_20   0x5d
#define MPU6050_REG_EXT_SENS_DATA_21   0x5e
#define MPU6050_REG_EXT_SENS_DATA_22   0x5f
#define MPU6050_REG_EXT_SENS_DATA_23   0x60
#define MPU6050_REG_I2C_SLV0_DO        0x63
#define MPU6050_REG_I2C_SLV1_DO        0x64
#define MPU6050_REG_I2C_SLV2_DO        0x65
#define MPU6050_REG_I2C_SLV3_DO        0x66
#define MPU6050_REG_I2C_MST_DELAY_CTRL 0x67
#define MPU6050_REG_SIGNAL_PATH_RESET  0x68
#define MPU6050_REG_USER_CTRL          0x6a
#define MPU6050_REG_PWR_MGMT_1         0x6b
#define MPU6050_REG_PWR_MGMT_2         0x6c
#define MPU6050_REG_FIFO_COUNTH        0x72
#define MPU6050_REG_FIFO_COUNTL        0x73
#define MPU6050_REG_FIFO_R_W           0x74
#define MPU6050_REG_WHO_AM_I           0x75

#define MPU6050_SLAVE_ADDR_AD0_LOW  0xd0
#define MPU6050_SLAVE_ADDR_AD0_HIGH 0xd2
#define SlaveAddr ((Handle->Init.SA0 == RESET) ? MPU6050_SLAVE_ADDR_AD0_LOW : MPU6050_SLAVE_ADDR_AD0_HIGH)

static ErrorStatus i2c_write(PalMPU6050_HandleTypeDef *Handle, uint8_t MemAddr, uint8_t Byte);
static ErrorStatus i2c_read_bytes(PalMPU6050_HandleTypeDef *Handle, uint8_t MemAddr, uint8_t *pBuffer, uint16_t Length);
static ErrorStatus reset_fifo(PalMPU6050_HandleTypeDef *Handle);

//
// @简介：将句柄的初始化参数设置为默认值
//        SA0        = RESET，从机地址=0xd0
//        SampleRate = 200Hz，采样率200Hz
//        DLPF       = 94Hz， 数字低通滤波器的截至频率为94Hz
//        AccelFsr   = 2g，   加速度计量程为2g
//        GyroFsr    = 2000， 陀螺仪量程为2000°/s
//        Tau        = 0.1
// @注意：仍需要手动填写的项目
//        I2Cx       - 用于和MPU6050进行通信的I2C接口的句柄
//
void PAL_MPU6050_InitHandle(PalMPU6050_HandleTypeDef *Handle)
{
	Handle->Init.SA0 = RESET;
	Handle->Init.SampleRate = 200;
	Handle->Init.MPU6050_DLPF = MPU6050_DLFP_94Hz_98Hz;
	Handle->Init.MPU6050_AccelFsr = MPU6050_ACCELSCALERANGE_2g;
	Handle->Init.MPU6050_GyroFsr = MPU6050_GYROSCALERANGE_2000;
	Handle->Init.Tau = 0.1;
	Handle->Init.Bias.AccelBias[0] = 0;
	Handle->Init.Bias.AccelBias[1] = 0;
	Handle->Init.Bias.AccelBias[2] = 0;
	Handle->Init.Bias.GyroBias[0] = 0;
	Handle->Init.Bias.GyroBias[1] = 0;
	Handle->Init.Bias.GyroBias[2] = 0;
	Handle->Init.Bias.TempBias = 0;
}

//
// @简介：对MPU6050进行初始化
// @参数：Handle - MPU6050的句柄
// @返回值：SUCCESS - 初始化成功，ERROR - 初始化失败
//
ErrorStatus PAL_MPU6050_Init(PalMPU6050_HandleTypeDef *Handle)
{
	PalI2C_HandleTypeDef *hi2c;
	
	Handle->ProcRunInterval = (uint32_t)(1.0 / Handle->Init.SampleRate * 1000);
	Handle->ProcNextExecTime = PAL_INVALID_TICK;
	Handle->FusionComputed = RESET;
	
	hi2c = Handle->Init.I2Cx;
	
	if(hi2c == 0)
	{
		return ERROR; // 用户未指定用于通信的I2C句柄
	}
	
	/* 设备复位 */
	CHECK(i2c_write(Handle, MPU6050_REG_PWR_MGMT_1, 0x80))
	PAL_Delay(100);
	
	
	/* 关闭睡眠模式，并将陀螺仪作为时钟来源 */
	CHECK(i2c_write(Handle, MPU6050_REG_PWR_MGMT_1, 0x01))
	
	
	/* 设置采样率 */
	uint32_t clockSpeed = 1000;
	
//	if(Handle->Init.MPU6050_DLPF == MPU6050_DLFP_260Hz_256Hz || Handle->Init.MPU6050_DLPF == MPU6050_DLFP_RESERVED)
//	{
//		clockSpeed = 1000; // 如果低通滤波器的类型为0或7，时钟频率降为1kHz
//	}
	CHECK(i2c_write(Handle, MPU6050_REG_SMPRT_DIV,  (uint8_t)round(clockSpeed * 1.0 / Handle->Init.SampleRate) - 1))
	
	Handle->SampleInterval = 1.0 / Handle->Init.SampleRate;
	
	
	/* 计算阿尔法的值 */
	Handle->Alpha = Handle->Init.Tau / (Handle->Init.Tau + 1.0 / Handle->Init.SampleRate);
	
	
	/* 设置数字低通滤波器的转折频率 */
	CHECK(i2c_write(Handle, MPU6050_REG_CONFIG,  Handle->Init.MPU6050_DLPF))
	
	
	/* 设置加速度传感器的量程 */
	CHECK(i2c_write(Handle, MPU6050_REG_ACCEL_CONFIG,  Handle->Init.MPU6050_AccelFsr))
	
	switch(Handle->Init.MPU6050_AccelFsr)
	{
		case MPU6050_ACCELSCALERANGE_2g:
			Handle->AccelScaleFactor = 1.0 / 16384; break;
		case MPU6050_ACCELSCALERANGE_4g:
			Handle->AccelScaleFactor = 1.0 / 8192; break;
		case MPU6050_ACCELSCALERANGE_8g:
			Handle->AccelScaleFactor = 1.0 / 4096; break;
		case MPU6050_ACCELSCALERANGE_16g:
			Handle->AccelScaleFactor = 1.0 / 2048; break;
	}
	
	
	/* 温度计量程固定，无需设置 */
	Handle->TempratureScaleFactor = 1.0 / 340;
	
	
	/* 设置陀螺仪的量程 */
	CHECK(i2c_write(Handle, MPU6050_REG_GYRO_CONFIG,  Handle->Init.MPU6050_GyroFsr));
	
	switch(Handle->Init.MPU6050_GyroFsr)
	{
		case MPU6050_GYROSCALERANGE_250:
			Handle->GyroScaleFactor = 1.0 / 131; break;
		case MPU6050_GYROSCALERANGE_500:
			Handle->GyroScaleFactor = 1.0 / 65.5; break;
		case MPU6050_GYROSCALERANGE_1000:
			Handle->GyroScaleFactor = 1.0 / 32.8; break;
		case MPU6050_GYROSCALERANGE_2000:
			Handle->GyroScaleFactor = 1.0 / 16.4; break;
	}
	
	/* 配置FIFO */
	CHECK(i2c_write(Handle, MPU6050_REG_FIFO_EN, 0xf8))  // 将陀螺仪、加速度计、温度传感器的数据推入FIFO
	
	/* 使能FIFO */
	CHECK(i2c_write(Handle, MPU6050_REG_USER_CTRL, 0x40))
	
	return SUCCESS;
}

#define PACKAGE_LENGTH 14   // 单个数据包的长度，GyroX(u16)+GyroY(u16)+GyroZ(u16)+AccelX(u16)+AccelY(u16)+AccelZ(u16)+temprature(u16)
#define FIFO_SIZE      1024 // FIFO的最大长度
#define HALF_FIFO_SIZE 512  // FIFO半满时的长度
#define DEG2RAD        0.01745329251994329576923690768489
#define RAD2DEG        57.295779513082320876798154814105

//
// @简介：  MPU6050进程函数，负责从传感器读取数据并进行处理
// @参数：  Handle - MPU6050的句柄
// @返回值：SUCCESS - 成功，ERROR - 失败
// 
ErrorStatus PAL_MPU6050_Proc(PalMPU6050_HandleTypeDef *Handle)
{
	/* 保证进程函数每 Handle->ProcRunInterval 毫秒执行一次 */
	if(Handle->ProcNextExecTime == PAL_INVALID_TICK)
	{
		Handle->ProcNextExecTime = PAL_GetTick();
	}
	
	if(Handle->ProcNextExecTime > PAL_GetTick())
	{
		return SUCCESS;
	}
	
	Handle->ProcNextExecTime += Handle->ProcRunInterval;
	
	
	/* 获取FIFO的长度 */
	uint8_t tmp[2];
	CHECK(i2c_read_bytes(Handle, MPU6050_REG_FIFO_COUNTH, tmp, 2))
	uint16_t fifoCount = (tmp[0] << 8) | tmp[1];
	
	if(fifoCount < PACKAGE_LENGTH) return SUCCESS; // 无数据需要被读取
	
	
	/* 如果FIFO的长度超过一半，检查溢出标志位 */
	if (fifoCount > HALF_FIFO_SIZE)
	{
		uint8_t intStatus;
		
		CHECK (i2c_read_bytes(Handle, MPU6050_REG_INT_STATUS, &intStatus, 1))
		
		/* 如果FIFO溢出则对FIFO进行重置 */
		if (intStatus & 0x10) {
			CHECK(reset_fifo(Handle))
			return SUCCESS;
		}
	}
	
	
	/* 将数据依次从FIFO中读出并进行处理 */
	uint8_t buffer[PACKAGE_LENGTH];
	
	uint32_t ii;
	
	while(fifoCount >= PACKAGE_LENGTH)
	{
		/* 从FIFO中读取一帧数据 */
		CHECK(i2c_read_bytes(Handle, MPU6050_REG_FIFO_R_W, buffer, PACKAGE_LENGTH)) 
		fifoCount -= PACKAGE_LENGTH;
		
		ii = 0;
		
		/* 从原始数据中计算加速度计的读数 */
		Handle->Cache.ax = (short)((buffer[ii+0] << 8) | buffer[ii+1]) * Handle->AccelScaleFactor - Handle->Init.Bias.AccelBias[0];
		Handle->Cache.ay = (short)((buffer[ii+2] << 8) | buffer[ii+3]) * Handle->AccelScaleFactor - Handle->Init.Bias.AccelBias[1];
		Handle->Cache.az = (short)((buffer[ii+4] << 8) | buffer[ii+5]) * Handle->AccelScaleFactor - Handle->Init.Bias.AccelBias[2];
		ii += 6;
		
		/* 从原始数据中计算温度计的读数 */
		Handle->Cache.temp = (short)((buffer[ii+0] << 8) | buffer[ii+1]) * Handle->TempratureScaleFactor + 36.53 - Handle->Init.Bias.TempBias;
		ii += 2;
		
		/* 从原始数据中计算陀螺仪的读数 */
		Handle->Cache.gx = (short)((buffer[ii+0] << 8) | buffer[ii+1]) * Handle->GyroScaleFactor - Handle->Init.Bias.GyroBias[0];
		Handle->Cache.gy = (short)((buffer[ii+2] << 8) | buffer[ii+3]) * Handle->GyroScaleFactor - Handle->Init.Bias.GyroBias[1];
		Handle->Cache.gz = (short)((buffer[ii+4] << 8) | buffer[ii+5]) * Handle->GyroScaleFactor - Handle->Init.Bias.GyroBias[2];
		ii += 6;
		
		
		/* 根据加速度计读数计算出roll_accel和pitch_accel */
		float roll_accel  = atan2(-Handle->Cache.ay, Handle->Cache.az) * RAD2DEG;
		float pitch_accel = -atan2( Handle->Cache.ax, Handle->Cache.az) * RAD2DEG;
		
		
		/* 进行传感器融合 */
		if(Handle->FusionComputed == RESET)
		{
			Handle->Cache.yaw   = 0;
		  Handle->Cache.roll  = roll_accel;
		  Handle->Cache.pitch = pitch_accel;
			
			Handle->FusionComputed = SET;
		}
		else
		{
			/* 计算偏航角 */
			/* yaw[k]=yaw[k-1]+gz*deltaT */
			Handle->Cache.yaw = Handle->Cache.yaw + Handle->Cache.gz * Handle->SampleInterval;
			
			/* 将偏航角yaw的范围限制在+-180度之间 */
			if(Handle->Cache.yaw >  180) Handle->Cache.yaw -= 360;
			if(Handle->Cache.yaw < -180) Handle->Cache.yaw += 360;
			
			
			/* 计算翻滚角 */
			/* 将根据加速度计算出的翻滚角roll_accel和上次计算得到的翻滚角roll的差值小于180度*/
			if(roll_accel-Handle->Cache.roll >  180) roll_accel -= 360;
			if(roll_accel-Handle->Cache.roll < -180) roll_accel += 360;
			
			/* 使用互补滤波器对传感器进行融合，以计算新的翻滚角roll的值 */
			Handle->Cache.roll  = Handle->Alpha * (Handle->Cache.roll  + Handle->Cache.gx * Handle->SampleInterval) + (1-Handle->Alpha) * roll_accel;
			
			/* 将翻滚角roll的范围限制在+-180度之间 */
			if(Handle->Cache.roll >  180) Handle->Cache.roll -= 360;
			if(Handle->Cache.roll < -180) Handle->Cache.roll += 360;
			
			
			/* 计算俯仰角 */
			/* 将根据加速度计算出的俯仰角pitch_accel和上次计算得到的俯仰角pitch的差值小于180度*/
			if(pitch_accel-Handle->Cache.pitch >  180) pitch_accel -= 360;
			if(pitch_accel-Handle->Cache.pitch < -180) pitch_accel += 360;
			
			/* 使用互补滤波器对传感器进行融合，以计算新的俯仰角pitch的值 */
			Handle->Cache.pitch = Handle->Alpha * (Handle->Cache.pitch + Handle->Cache.gy * Handle->SampleInterval) + (1-Handle->Alpha) * pitch_accel;
			
			/* 将俯仰角pitch的范围限制在+-180度之间 */
			if(Handle->Cache.pitch >  180) Handle->Cache.pitch -= 360;
			if(Handle->Cache.pitch < -180) Handle->Cache.pitch += 360;
		}
	}
	
	return SUCCESS;
}

//
// @简介：  获取加速度传感器x轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：x轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.ax;
}

//
// @简介：  获取加速度传感器y轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：y轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.ay;
}

//
// @简介：  获取加速度传感器z轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：z轴向加速度，单位g，其中g为重力加速度
//
float PAL_MPU6050_GetAccelZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.az;
}

//
// @简介：  获取陀螺仪传感器x轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：x轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroX(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gx;
}

//
// @简介：  获取陀螺仪传感器y轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：y轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroY(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gy;
}

//
// @简介：  获取陀螺仪传感器z轴方向的读数
// @参数：  Handle - MPU6050的句柄
// @返回值：z轴向角速度，单位°/s
//
float PAL_MPU6050_GetGyroZ(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.gz;
}

//
// @简介：  获取偏航角
// @参数：  Handle - MPU6050的句柄
// @返回值：偏航角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetYaw(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.yaw;
}

//
// @简介：  获取翻滚角
// @参数：  Handle - MPU6050的句柄
// @返回值：翻滚角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetRoll(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.roll;
}

//
// @简介：  获取俯仰角
// @参数：  Handle - MPU6050的句柄
// @返回值：俯仰角，单位°，范围-180° ~ +180°
//
float PAL_MPU6050_GetPitch(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.pitch;
}

//
// @简介：  获取温度
// @参数：  Handle - MPU6050的句柄
// @返回值：温度，单位℃
//
float PAL_MPU6050_GetTemperature(PalMPU6050_HandleTypeDef *Handle)
{
	return Handle->Cache.temp;
}

static ErrorStatus i2c_write(PalMPU6050_HandleTypeDef *Handle, uint8_t MemAddr, uint8_t Byte)
{
	return PAL_I2C_MemWrite(Handle->Init.I2Cx, SlaveAddr, MemAddr, I2C_MEMSIZE_8BIT, &Byte, 1);
}

static ErrorStatus i2c_read_bytes(PalMPU6050_HandleTypeDef *Handle, uint8_t MemAddr, uint8_t *pBuffer, uint16_t Length)
{
	return PAL_I2C_MemRead(Handle->Init.I2Cx, SlaveAddr, MemAddr, I2C_MEMSIZE_8BIT, pBuffer, Length);
}

static ErrorStatus reset_fifo(PalMPU6050_HandleTypeDef *Handle)
{
	Handle->FusionComputed = RESET;
	CHECK(i2c_write(Handle, MPU6050_REG_FIFO_EN, 0))      // 关闭所有FIFO项目
	CHECK(i2c_write(Handle, MPU6050_REG_USER_CTRL, 0))    // 关闭FIFO开关
	CHECK(i2c_write(Handle, MPU6050_REG_USER_CTRL, 0x04)) // 复位FIFO
	CHECK(i2c_write(Handle, MPU6050_REG_USER_CTRL, 0x40)) // 使能FIFO
	PAL_Delay(50);
	CHECK(i2c_write(Handle, MPU6050_REG_FIFO_EN, 0xf8))   // 恢复FIFO项目
	return SUCCESS;
}
