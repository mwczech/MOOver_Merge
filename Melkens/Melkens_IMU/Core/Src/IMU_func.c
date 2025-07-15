
#define NUCLEO_F411RE 1


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include "lsm6dsr_reg.h"
#include "lis3mdl_reg.h"
#include "main.h"
#include "IMU_func.h"
#include "UartHandler.h"
#include "math.h"
#include "MadgwickAHRS.h"
#include "MagnetsHandler.h"
#include "ConnectivityHandler.h"
#include "CRC16.h"
//assign the structures
//UART_HandleTypeDef huart1;

/* Private macro -------------------------------------------------------------*/

//#define SENSOR_BUS hi2c1
#define dFILTER
#define dFILTER_COUNT 4
#define dFILTRATED_TABLE_LEN dFIFO_DEPTH/dFILTER_COUNT
#define dGRAVITY_COMPENSATION_STEPS 100
#define SAMPLING_RATE 1 /* 1ms */
#define POSITIVE_LIMIT_ACC 1.0f
#define NEGATIVE_LIMIT_ACC -1.0f
#define RAD_TO_DEG 57.295779513082320876798154814105

//#define DELIMITER 0x0A0D //"\r\n"

I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c4;
/* Private variables ---------------------------------------------------------*/

//AHRS=============

// Soft iron error compensation matrix
float mag_softiron_matrix[3][3] = { { 1.0, 0.0, 0.0 },
                                    { 0.0, 1.0, 0.0 },
                                    { 0.0, 0.0, 1.0 } };
float pitch, yaw, roll = 3.14;

float abias[3] = {0, 0, 0}, gbias[3] = {0, 0, 0};
float ax, ay, az, gx, gy, gz, mx, my, mz; // variables to hold latest sensor data values
float magBias[3] = {-166, 1459, 1005.5}; //for hard iron calibration

int calibration_samples = 1000;
int calibration_sample_count = 0;
float gxSum = 0, gySum = 0, gzSum = 0, magxSum = 0, magySum = 0, magzSum = 0;

float magXinit = 0, magYinit = 0, magZinit = 0;
float first_compass_reading = 0;

bool AHRS_ready = false;

float gyroConversionFactor = 0.000285;//for calculating dps(degrees per second) from gyro output


uint8_t whoamI;
uint8_t whoamI2;
uint8_t GravCompensationCounter = 0;
int32_t CompensationX,CompensationY,CompensationZ;


Pmb2ImuFrame_t Pmb2ImuFrame;
Imu2PmbFrame_t Imu2PmbFrame;
Imu2EspFrame_t Imu2EspFrame;
Imu2PCFrame_t Imu2PCFrame;//debug communication

int16_t Timer1000ms = 10000;

stmdev_lsm_ctx_t dev_ctx;
stmdev_lis_ctx_t dev_ctx2;

static uint8_t tx_buffer[100];

char txt_ev[] = {"EV\r\n"};
char txt_xh[] = {"XH\r\n"};
char txt_xl[] = {"XL\r\n"};
char txt_yh[] = {"YX\r\n"};
char txt_yl[] = {"YL\r\n"};
char txt_zh[] = {"ZH\r\n"};
char txt_zl[] = {"ZL\r\n"};

float dt = .08f;
bool IsPeripheralReady = false;
bool DataReady = false;
bool ResetState = true;
uint8_t IMUInitLoops = 50;

struct convdata
{
	int16_t Fnew_X;
	int16_t Fold_X;
	int16_t Fnew_Y;
	int16_t Fold_Y;
	int16_t Fnew_Z;
	int16_t Fold_Z;
	int16_t Meas_X;
	int16_t Meas_Y;
	int16_t Meas_Z;
}giromeas, accelmeas;

typedef struct AxisData
{
	int16_t Xaxis;
	int16_t Yaxis;
	int16_t Zaxis;

}AxisData_t;

typedef enum device_type{
	LSM6DSR = 0,
	LIS3MDL = 1,
	Device_NumOf
}device_type_t;

AxisData_t AccelerationData[dFIFO_DEPTH];
AxisData_t GyroscopeData[dFIFO_DEPTH];

static AxisData_t AccelerationFiltered[dFILTRATED_TABLE_LEN];
static AxisData_t GyroscopeFiltered[dFILTRATED_TABLE_LEN];

int16_t AccGlobalX;
int16_t AccGlobalY;
int16_t AccGlobalZ;

int16_t AccXCompensation;
int16_t AccYCompensation;
int16_t AccZCompensation;

float YaxisTot;
float XaxisTot;

float giroAngle_X;
float giroAngle_Y;
float giroAngle_Z;
static float pusher_position[3];

//arrays
static float acceleration_mg[3];
volatile float angular_velocity[3];
static float angular_rate_mdps[3];
int16_t angular_rate_dps[3];
int16_t data_raw_acceleration[3];
int16_t data_raw_acceleration_debug[3];
int16_t data_raw_magnetic[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature[3];
static float velocity2[3] = {0.0f, 0.0f, 0.0f};
static float prev_velocity2[3] = {0.0f, 0.0f, 0.0f};
static float magnetic_mG[3];

int16_t  AccDataTemp[3];
int16_t  GyroDataTemp[3];
int16_t FlushData;
uint16_t data_gyr_count= 0;
uint16_t numacc_save;
uint16_t numgyro_save;
uint32_t timestamp_acc;
uint32_t timestamp_acc_prev;
uint32_t delta_acc;

uint16_t AccCounter = 0;
uint16_t GyroCounter = 0;
lsm6dsr_fifo_tag_t reg_tag;
uint16_t num;


float roll2_sqrt;
float roll2;
float pitch2;
float accelData2[3];
uint8_t rst;


/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void lsm6dsr_Init(void);
void lis3mdl_Init(void);

void IMU_i2c_init(void)
{
	  lsm6dsr_Init();

	  //lis3mdl_Init();
}

void lis3mdl_Init(void)
{
	  dev_ctx2.handle = &hi2c4;
	  dev_ctx2.write_reg = platform_write_lis3mdl;
	  dev_ctx2.read_reg = platform_read_lis3mdl;
	  rst = 1;

	  HAL_Delay(BOOT_TIME);
	  lis3mdl_reset_set(&dev_ctx2, PROPERTY_ENABLE);
	  whoamI = 0;
	  do {
		  lis3mdl_reset_get(&dev_ctx2, &rst);
	  } while (rst);

	  lis3mdl_device_id_get(&dev_ctx2, &whoamI2);

	  if (whoamI2 != LIS3MDL_ID)
	    while (1); /*manage here device not found */

	  /* Enable Block Data Update */
	  lis3mdl_block_data_update_set(&dev_ctx2, PROPERTY_ENABLE);
	  /* Set Output Data Rate */
	  lis3mdl_data_rate_set(&dev_ctx2, LIS3MDL_HP_40Hz);
	  /* Set full scale */
	  lis3mdl_full_scale_set(&dev_ctx2, LIS3MDL_4_GAUSS);
	  /* Enable temperature sensor */
	  lis3mdl_temperature_meas_set(&dev_ctx2, PROPERTY_ENABLE);
	  /* Set device in continuous mode */
	  lis3mdl_operating_mode_set(&dev_ctx2, LIS3MDL_CONTINUOUS_MODE);


}

void lsm6dsr_Init(void)
{
	  dev_ctx.handle = &hi2c2;
	  dev_ctx.write_reg = platform_write;
	  dev_ctx.read_reg = platform_read;

	  lsm6dsr_pin_int1_route_t int1_route;
	  lsm6dsr_pin_int2_route_t int2_route;

	  HAL_Delay(BOOT_TIME);

	  lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);

	  do {
	    lsm6dsr_reset_get(&dev_ctx, &rst);
	  } while (rst);

	  lsm6dsr_device_id_get(&dev_ctx, &whoamI);

	  if (whoamI != LSM6DSR_ID)
	  {
		  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);//green
		  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);//yellow
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);//red
		  while (1); /*manage here device not found */
	  }
	  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);//green

	  lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);
	  /* Enable Block Data Update */
	  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
	  lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_500dps);

	  //lsm6dsr_timestamp_set(&dev_ctx, PROPERTY_ENABLE);

	  lsm6dsr_fifo_watermark_set(&dev_ctx, dFIFO_DEPTH * 2);


	  lsm6dsr_fifo_xl_batch_set(&dev_ctx, LSM6DSR_XL_BATCHED_AT_417Hz);
	  lsm6dsr_fifo_gy_batch_set(&dev_ctx, LSM6DSR_GY_BATCHED_AT_417Hz);

	  lsm6dsr_fifo_mode_set(&dev_ctx, LSM6DSR_STREAM_MODE);

	  /* Enable drdy 75 μs pulse: uncomment if interrupt must be pulsed */
	  lsm6dsr_data_ready_mode_set(&dev_ctx, LSM6DSR_DRDY_PULSED);
	  /* Uncomment if interrupt generation on Free Fall INT1 pin */
	  lsm6dsr_pin_int1_route_get(&dev_ctx, &int1_route);
	  int1_route.int1_ctrl.int1_fifo_th = PROPERTY_ENABLE;
	  lsm6dsr_pin_int1_route_set(&dev_ctx, &int1_route);
	  /* Uncomment if interrupt generation on Free Fall INT2 pin */
	  lsm6dsr_pin_int2_route_get(&dev_ctx, &int2_route);
	  int2_route.int2_ctrl.int2_fifo_th = PROPERTY_ENABLE;
	  lsm6dsr_pin_int2_route_set(&dev_ctx, &int2_route);

	  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_416Hz);
	  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_416Hz);

	  lsm6dsr_gy_filter_lp1_set(&dev_ctx, PROPERTY_ENABLE);

	  lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_200);
	  lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);


}

float IMU_12_5_to_angle(int16_t lsb)
{
  return ((float_t)lsb) / .08;
}

float angle_from_fs2000(int16_t lsb)
{
	return ((float_t)lsb) * .07f / .08f;
	//0.07 taken from 2000dps ->70mdps/LSB
	//0.08 taken from 12,5Hz
}

float angular_velocity_from_dps(int16_t lsb)
{
	return ((float_t)lsb) * .01745f;
}

int32_t  platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{

	HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg,
		             I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

	 return 0;
}

int32_t  platform_write_lis3mdl(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{

	HAL_I2C_Mem_Write(handle, LIS3MDL_I2C_ADD_H, reg,
		             I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

	 return 0;
}

int32_t  platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{

	HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg,
					I2C_MEMADDRESS_SIZE_8BIT, bufp, len, 1000);

/* Dummy solution, function suppose to return anything  */
  return 0;
}

int32_t  platform_read_lis3mdl(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{

	HAL_I2C_Mem_Read(handle, LIS3MDL_I2C_ADD_H, reg,
					I2C_MEMADDRESS_SIZE_8BIT, bufp, len, 1000);

/* Dummy solution, function suppose to return anything  */
  return 0;
}

static void platform_delay(uint32_t ms)
{

  HAL_Delay(ms);
}

static void platform_init(void)
{
#if defined(STEVAL_MKI109V3)
  TIM3->CCR1 = PWM_3V3;
  TIM3->CCR2 = PWM_3V3;
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_Delay(1000);
#endif
}

void IMU_StoreAccData(int16_t *Value, int16_t Index)
{
	AccelerationData[Index].Xaxis = Value[0];
	AccelerationData[Index].Yaxis = Value[1];
	AccelerationData[Index].Zaxis = Value[2];
}

void IMU_StoreGyroData(int16_t *Value, int16_t Index)
{
	GyroscopeData[Index].Xaxis = Value[0];
	GyroscopeData[Index].Yaxis = Value[1];
	GyroscopeData[Index].Zaxis = Value[2];
}

void IMU_SetDataReadyFlag(void)
{
	DataReady = true;
}

void IMU_Filtration(void)
{
	int32_t AccX_Accu,AccY_Accu,AccZ_Accu,GyrX_Accu,GyrY_Accu,GyrZ_Accu;
	uint16_t IterationStep = 0;
	for(uint16_t indexI = 0; indexI < dFILTRATED_TABLE_LEN; indexI++)
	{
		AccX_Accu = 0;
		AccY_Accu = 0;
		AccZ_Accu = 0;
		GyrX_Accu = 0;
		GyrY_Accu = 0;
		GyrZ_Accu = 0;
		/* Generate one sample using filtration */
		for(uint16_t indexJ = 0; indexJ < dFILTER_COUNT; indexJ++)
		{
			/* Iterate through samples collected from FIFO */
			AccX_Accu += AccelerationData[IterationStep + indexJ].Xaxis;
			AccY_Accu += AccelerationData[IterationStep + indexJ].Yaxis;
			AccZ_Accu += AccelerationData[IterationStep + indexJ].Zaxis;
			GyrX_Accu += GyroscopeData[IterationStep + indexJ].Xaxis;
			GyrY_Accu += GyroscopeData[IterationStep + indexJ].Yaxis;
			GyrZ_Accu += GyroscopeData[IterationStep + indexJ].Zaxis;

		}
		CompensationX = 0;
		CompensationY = 0;
		CompensationZ = 0;
		AccelerationFiltered[indexI].Xaxis = AccX_Accu / dFILTER_COUNT - CompensationX;
		AccelerationFiltered[indexI].Yaxis = AccY_Accu / dFILTER_COUNT - CompensationY;
		AccelerationFiltered[indexI].Zaxis = AccZ_Accu / dFILTER_COUNT - CompensationZ;
		GyroscopeFiltered[indexI].Xaxis = GyrX_Accu / dFILTER_COUNT;
		GyroscopeFiltered[indexI].Yaxis = GyrY_Accu / dFILTER_COUNT;
		GyroscopeFiltered[indexI].Zaxis = GyrZ_Accu / dFILTER_COUNT;


		AccGlobalX = AccelerationFiltered[indexI].Xaxis;
		AccGlobalY = AccelerationFiltered[indexI].Yaxis;
		AccGlobalZ = AccelerationFiltered[indexI].Zaxis;

		IterationStep += dFILTER_COUNT;
	}
}

void IMU_ResetDataReady(void)
{
	DataReady = false;
}

void IMU_GravityCompensation(void)
{

	if( GravCompensationCounter < dGRAVITY_COMPENSATION_STEPS )
	{
		GravCompensationCounter += dFIFO_DEPTH;
		for(int i=0; i<dFIFO_DEPTH; i++)
		{
			CompensationX+=AccelerationData[i].Xaxis;
			CompensationY+=AccelerationData[i].Yaxis;
			CompensationZ+=AccelerationData[i].Zaxis;
		}
	}
	else
	{
		/* Gravity compensation finished */
		CompensationX = CompensationX / dGRAVITY_COMPENSATION_STEPS;
		CompensationY = CompensationY / dGRAVITY_COMPENSATION_STEPS;
		CompensationZ = CompensationZ / dGRAVITY_COMPENSATION_STEPS;

		ResetState = false;
	}

}

void IMU_InitLoopTick(void)
{
	IMUInitLoops--;
}


void IMU_CollectMagnetometer(void)
{
    uint8_t reg;
    /* Read output only if new value is available */
    lis3mdl_mag_data_ready_get(&dev_ctx2, &reg);

    if (reg) {
        /* Read magnetic field data */
        lis3mdl_magnetic_raw_get(&dev_ctx2, data_raw_magnetic);
        magnetic_mG[0] = 1000 * lis3mdl_from_fs16_to_gauss(
                           data_raw_magnetic[0]);
        magnetic_mG[1] = 1000 * lis3mdl_from_fs16_to_gauss(
                           data_raw_magnetic[1]);
        magnetic_mG[2] = 1000 * lis3mdl_from_fs16_to_gauss(
                           data_raw_magnetic[2]);
    }


}
void IMU_CollectFromFIFO(void)
{
AccCounter = 0;
GyroCounter = 0;

  lsm6dsr_fifo_data_level_get(&dev_ctx, &num);
	numacc_save = num * 2;
	if(num == dFIFO_DEPTH * 2)
	{
		AccCounter = 0;
		GyroCounter = 0;
		//num = num;
		while(num>0)
		{
			lsm6dsr_fifo_sensor_tag_get(&dev_ctx, &reg_tag);
			switch (reg_tag) {
					  case LSM6DSR_XL_NC_TAG:
						lsm6dsr_fifo_out_raw_get(&dev_ctx, &AccDataTemp);
						IMU_StoreAccData(&AccDataTemp, AccCounter );
						AccCounter++;
						break;

					  case LSM6DSR_GYRO_NC_TAG:
						lsm6dsr_fifo_out_raw_get(&dev_ctx, &GyroDataTemp);
						IMU_StoreGyroData(&GyroDataTemp, GyroCounter );
						GyroCounter++;
						break;

					  default:
						lsm6dsr_fifo_out_raw_get(&dev_ctx, &FlushData);
						break;
			}
			num--;
		}
	}
}

void IMU_Perform1ms(void){
	if(Timer1000ms > 0){
		Timer1000ms -= 1;
	}

}

bool IMU_Perform(void)
{
	if( UartHandler_IsDataReceived(Uart_PMB) )
	{
		UartHandler_GetRxBuffer(Uart_PMB, (uint8_t*)&Pmb2ImuFrame, sizeof(Pmb2ImuFrame_t));
		/* Process data */
		if( Pmb2ImuFrame.crc == CRC16((uint8_t*)&Pmb2ImuFrame, sizeof(Pmb2ImuFrame_t) - sizeof(Pmb2ImuFrame.crc)) ){
			//todo: [PM] update data from received structure
            // = Pmb2ImuFrame.motorRightRotation;
			// = Pmb2ImuFrame.motorRightRotation;

			/////////////////////////////////////////////////////////////////////////////
			// Send to Esp
			//todo: [PM] update this structure before sending
			Imu2EspFrame.pmbConnection = true; 
            Imu2EspFrame.batteryVoltage = Pmb2ImuFrame.batteryVoltage;
			Imu2EspFrame.magnetBarStatus = MagnetsHandler_GetSatus();
			Imu2EspFrame.motorRightSpeed = Imu2PmbFrame.motorRightSpeed;
			Imu2EspFrame.motorLeftSpeed = Imu2PmbFrame.motorLeftSpeed;
			Imu2EspFrame.adcCurrent = Pmb2ImuFrame.adcCurrent;
			Imu2EspFrame.thumbleCurrent = Pmb2ImuFrame.thumbleCurrent;
			Imu2EspFrame.crcImu2PmbErrorCount = Pmb2ImuFrame.crcImu2PmbErrorCount;
			Imu2EspFrame.crc = CRC16((uint8_t*)&Imu2EspFrame, sizeof(Imu2EspFrame_t) - sizeof(Imu2EspFrame.crc));
			
			UartHandler_SendMessage(Uart_ConnectivityESP, (char*)&Imu2EspFrame, sizeof(Imu2EspFrame_t));
			Timer1000ms = 1000;
		}
		else{
			Imu2EspFrame.crcPmb2ImuErrorCount++;
		}

		UartHandler_ReloadReceiveChannel(Uart_PMB);
		LL_USART_Enable(USART2);

	}
	else{
		
		if( Timer1000ms <= 0 ){
			Imu2EspFrame.pmbConnection = false; 
			Imu2EspFrame.crc = CRC16((uint8_t*)&Imu2EspFrame, sizeof(Imu2EspFrame_t) - sizeof(Imu2EspFrame.crc));
			UartHandler_SendMessage(Uart_ConnectivityESP, (char*)&Imu2EspFrame, sizeof(Imu2EspFrame_t));
			Timer1000ms = 1000;
		}

	}

	if( DataReady )
	{
		DataReady = false;
		IMU_CollectFromFIFO();
		//IMU_CollectMagnetometer();

		if( IsPeripheralReady )
		{
			if (ResetState)
			{
				/* Compensate gravity influence on Accelerometer redings */
				IMU_GravityCompensation();
			}
			else
			{
				IMU_Filtration();
				/* Calculate position of device from filtered acc values */

				IMU_AHRS_Calculation();
				//if(AHRS_ready)
				//IMU_RouteCalculation();

				//IMU_SendData();


			}
		}
		else
		{
			if( IMUInitLoops == 0 )
			{
				IsPeripheralReady = true;

			}
			else{
				IMU_InitLoopTick();
			}
		}
		IMU_ResetDataReady();
		return true;
	}
	return false;
}
uint16_t MagnetsLow;
uint16_t MagnetsHigh;

void IMU_SendDataToPMB(void){
    //todo: [PM] update this structure before sending
    // Imu2PmbFrame.motorRightSpeed = 
	// Imu2PmbFrame.motorLeftSpeed =
	// ...

	Imu2PmbFrame.crc = CRC16((uint8_t*)&Imu2PmbFrame, sizeof(Imu2PmbFrame_t) - sizeof(Imu2PmbFrame.crc));
		
	UartHandler_SendMessage(Uart_PMB, (char*)&Imu2PmbFrame, sizeof(Imu2PmbFrame));
}

void IMU_SendDataToPC(void){
    //todo: [PM] update this structure before sending
    // Imu2PCFrame.motorRightSpeed =
	// Imu2PCFrame.motorLeftSpeed =
	// ...

	//Imu2PCFrame.crc = CRC16((uint8_t*)&Imu2PCFrame, sizeof(Imu2PCFrame_t) - sizeof(Imu2PCFrame.crc));
	Imu2PCFrame.crc = '\n\r';

	UartHandler_SendMessage(Uart_3, (char*)&Imu2PCFrame, sizeof(Imu2PCFrame));
}

void IMU_AHRS_Calculation(void){

		gx = GyroscopeFiltered[0].Xaxis * gyroConversionFactor + gbias[0];   // Convert to degrees per seconds, remove gyro biases
	    gy = GyroscopeFiltered[0].Yaxis * gyroConversionFactor + gbias[1];
	    gz = GyroscopeFiltered[0].Zaxis * gyroConversionFactor + gbias[2];

	    ax = AccelerationFiltered[0].Xaxis / 16000 + abias[0];   // Convert to g's, remove accelerometer biases
	    ay = AccelerationFiltered[0].Yaxis / 16000 + abias[1];
	    az = AccelerationFiltered[0].Zaxis / 16000 + abias[2];

	    mx = 0;//magnetic_mG[0] + magBias[0];// Convert to Gauss and correct for calibration
	    my = 0;//magnetic_mG[1] + magBias[1];
	    mz = 0;//magnetic_mG[2] + magBias[2];

	    // mx=x;// =x*mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];     // Convert to Gauss and correct for calibration
	    // my=y;// =x*mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
	    // mz=z;// =x*mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

	    if(calibration_sample_count < calibration_samples)
	    {
	    	HAL_GPIO_WritePin(GPIOB, LED2_Pin, GPIO_PIN_SET);//yellow

	    	magxSum += mx;
			magySum += my;
			magzSum += mz;

			gxSum += gx;
			gySum += gy;
			gzSum += gz;

	    	calibration_sample_count++;

			if(calibration_sample_count == calibration_samples)
			{
				gbias[0] = -gxSum/calibration_samples;
				gbias[1] = -gySum/calibration_samples;
				gbias[2] = -gzSum/calibration_samples;

				magXinit = magxSum/calibration_samples;
				magYinit = magySum/calibration_samples;
				magZinit = magzSum/calibration_samples;

				first_compass_reading = atan2(magYinit, magXinit)-3.1415;

				roll = first_compass_reading;

				float cy = cos(yaw * 0.5);
				float sy = sin(yaw * 0.5);
				float cp = cos(pitch * 0.5);
				float sp = sin(pitch * 0.5);
				float cr = cos(roll * 0.5);
				float sr = sin(roll * 0.5);

				q0 = sr * cp * cy - cr * sp * sy;
				q1 = cr * sp * cy + sr * cp * sy;
				q2 = cr * cp * sy - sr * sp * cy;
				q3 = cr * cp * cy + sr * sp * sy;


//				last_enco_left_val = EncoderHandler_GetEncoderValue(LEFT_ENCODER);
//				last_enco_right_val = EncoderHandler_GetEncoderValue(RIGHT_ENCODER);

				AHRS_ready = true;
				HAL_GPIO_WritePin(GPIOB, LED2_Pin, GPIO_PIN_RESET);//yellow


			}
	    }
	    else
	    {
	    	//thresholds for gyro

	    	if(gx < 0.002 && gx > -0.002)
	    		gx = 0;

	    	if(gy < 0.002 && gy > -0.002)
	    		gy = 0;

	    	if(gz < 0.002 && gz > -0.002)
	    		gz = 0;

	    	MadgwickAHRSupdate(gx, gy, -gz, ax, ay, az, mx, my, mz);

	    	 // Roll (x-axis rotation)
	    	//q0  q1  q2  q3
	    	//x   y   z   w
		   float sinr_cosp = 2 * (q3 * q0 + q1 * q2);
		   float cosr_cosp = 1 - 2 * (q0 * q0 + q1 * q1);
		   roll = atan2(sinr_cosp, cosr_cosp);

		   // Pitch (y-axis rotation)
		   float sinp = 2 * (q3 * q1 - q2 * q0);
		   if (fabs(sinp) >= 1)
			   pitch = copysign(3.1415 / 2, sinp); // Use 90 degrees if out of range
		   else
			   pitch = asin(sinp);

		   // Yaw (z-axis rotation)
		   float siny_cosp = 2 * (q3 * q2 + q0 * q1);
		   float cosy_cosp = 1 - 2 * (q1 * q1 + q2 * q2);
		   yaw = atan2(siny_cosp, cosy_cosp);
	    }

	// MadgwickQuaternionUpdate(-ay, -ax, az, gy*PI/180.0f, gx*PI/180.0f, -gz*PI/180.0f, mx, my, mz);
	 //MahonyQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f, mx, my, mz);
}

float getRobotAngle(void)
{
	return roll;
}

//void IMU_RouteCalculation()
//{
//	int increment_L = EncoderHandler_GetEncoderValue(LEFT_ENCODER) - last_enco_left_val;
//	int increment_R = EncoderHandler_GetEncoderValue(RIGHT_ENCODER) - last_enco_right_val;
//
//	if(increment_L < -10)
//		increment_L = 1;
//	else if(increment_L > 10)
//		increment_L = -1;
//
//	if(increment_R < -10)
//		increment_R = 1;
//	else if(increment_R > 10)
//		increment_R = -1;
//
//	last_enco_left_val = EncoderHandler_GetEncoderValue(LEFT_ENCODER);
//	last_enco_right_val = EncoderHandler_GetEncoderValue(RIGHT_ENCODER);
//
//	left_wheel_distance += increment_L;
//	right_wheel_distance += increment_R;
//
//	wheel_distane_diff = left_wheel_distance - right_wheel_distance;
//	moover_angle_encoders = 3.1415 / 227 * wheel_distane_diff;
//
//	moover_velocity = (increment_L + increment_R) * 0.5;//pomnożyć przez jakąć stałą żeby się dystans zgadzał
//
//	velocity_x = moover_velocity * cos(roll - first_compass_reading);
//	velocity_y = moover_velocity * sin(roll - first_compass_reading);
//
//	X_POSITION_AHRS += velocity_x;
//	Y_POSITION_AHRS += velocity_y;
//
//	velocity_x = moover_velocity * cos(moover_angle_encoders);
//	velocity_y = moover_velocity * sin(moover_angle_encoders);
//
//	X_POSITION_ENCO += velocity_x;
//	Y_POSITION_ENCO += velocity_y;
//
//}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00701F6B;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(I2C_FASTMODEPLUS_I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00701F6B;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(I2C_FASTMODEPLUS_I2C4);
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

void setLeftWheelSpeed(int16_t speed)
{
	if(AHRS_ready)
		Imu2PmbFrame.motorLeftSpeed = speed;
}

void setRightWheelSpeed(int16_t speed)
{
	if(AHRS_ready)
		Imu2PmbFrame.motorRightSpeed = speed;
}

int16_t getLeftWheelSpeed(void)
{
	return Imu2PmbFrame.motorLeftSpeed;
}

int16_t getRightWheelSpeed(void)
{
	return Imu2PmbFrame.motorRightSpeed;
}

void setDebugDataPoint1(uint16_t x, uint16_t y)
{
	Imu2PCFrame.Xpos1 = x;
	Imu2PCFrame.Ypos1 = y;
}

void setDebugDataPoint2(uint16_t x, uint16_t y)
{
	Imu2PCFrame.Xpos2 = x;
	Imu2PCFrame.Ypos2 = y;
}

uint16_t getRightEncoder(void)
{
	return Pmb2ImuFrame.motorRightRotation;
}

uint16_t getLeftEncoder(void)
{
	return Pmb2ImuFrame.motorLeftRotation;
}

bool isAhrsReady(void)
{
	return AHRS_ready;
}

void setThumbleSpeed(uint16_t speed)
{
	if(AHRS_ready)
		Imu2PmbFrame.motorThumbleSpeed = speed;
}

