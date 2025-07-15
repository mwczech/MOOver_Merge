/*
 * DataTypes.h
 *
 *  Created on: Dec 17, 2022
 *      Author: pawelton
 */
#include"MessageTypes.h"


#ifndef INC_DATATYPES_H_
#define INC_DATATYPES_H_

#define UART1_RX_MESSAGE_LEN 		sizeof(Esp2ImuFrame_t) /* E;RRRRR;LLLLL;33 -> Encoder;RightWheel int16, LeftWheel int16, Dummy CRC */
#define UART2_RX_MESSAGE_LEN 		sizeof(Pmb2ImuFrame_t) /* GET_ENCO -> Getter message, always constant length of 8 bytes */
#define UART3_RX_MESSAGE_LEN 		15 		/* W;RRR;LLL;33\n -> WheelSpeed; RightWheel int8, LeftWheel int8, Dummy CRC */
#define UART5_RX_MESSAGE_LEN 		12

#define UART1_TX_MESSAGE_LEN 		sizeof(Imu2EspFrame_t) /* E;RRRRR;LLLLL;33 -> Encoder;RightWheel int16, LeftWheel int16, Dummy CRC */
#define UART2_TX_MESSAGE_LEN 		sizeof(Imu2PmbFrame_t) /* Data from IMU to PMB consisting magnets, roll, pitch and yaw  */
#define UART3_TX_MESSAGE_LEN 		24 		/* W;RRR;LLL;33\n -> WheelSpeed; RightWheel int8, LeftWheel int8, Dummy CRC */
#define UART5_TX_MESSAGE_LEN 		15

#define ENCO_TEMP	12

/* Getter from encoder */
#define ENCODER_GET_VALUE 5

/* Setter of wheels */
#define SET_WHEELS_SPEED 6

/* Specific characters */
//#define MESSAGE_CRC "33"
//#define MESSAGE_SEPARATOR ";"
//#define MESSAGE_NEW_LINE_CHARACTER "\n"

#endif /* INC_DATATYPES_H_ */
