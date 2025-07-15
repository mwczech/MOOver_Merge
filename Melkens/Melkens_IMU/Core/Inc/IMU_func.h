/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"
#include "stdbool.h"
#include "MessageTypes.h"

/**************SETTINGS*************/

/***********************************/


#define    BOOT_TIME            10 //ms
#define I2C_MEMADDRESS_SIZE_8BIT     (0x00000001U)
#define dFIFO_DEPTH 4

extern Imu2EspFrame_t Imu2EspFrame;

int32_t  platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
int32_t  platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
int32_t  platform_write_lis3mdl(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
int32_t  platform_read_lis3mdl(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);

void IMU_i2c_init(void);

void MX_I2C2_Init(void);
void MX_I2C4_Init(void);


void lsm6dsr_activity(void);
void lsm6dsr_read_data_polling(void);
void lsm6dsr_orientation(void);
void lsm6dsr_compressed_fifo(void);

float angle_from_fs2000(int16_t lsb);
float angular_velocity_from_dps(int16_t lsb);

float getRobotAngle(void);
uint16_t getRightEncoder(void);
uint16_t getLeftEncoder(void);

void setLeftWheelSpeed(int16_t speed);
void setRightWheelSpeed(int16_t speed);

void setDebugDataPoint1(uint16_t x, uint16_t y);
void setDebugDataPoint2(uint16_t x, uint16_t y);

float IMU_12_5_to_angle(int16_t lsb);
void IMU_StoreAccData(int16_t *Value, int16_t Index);
void IMU_StoreGyroData(int16_t *Value, int16_t Index);
void IMU_Perform1ms(void);
bool IMU_Perform(void);
void IMU_SetDataReadyFlag(void);
void IMU_ResetDataReady(void);
void IMU_InitLoopTick(void);

void IMU_SendDataToPMB(void);
void IMU_SendDataToPC(void);

void IMU_AHRS_Calculation(void);
void IMU_RouteCalculation(void);
bool isAhrsReady(void);

int16_t getRightWheelSpeed(void);
int16_t getLeftWheelSpeed(void);

void setThumbleSpeed(uint16_t speed);
