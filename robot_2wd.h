// 
// Base struct for 2WD robot
//
//
// robocraft.ru
//

#ifndef _ROBOT_2WD_H_
#define _ROBOT_2WD_H_

#include "orcp2.h"

#define MOTORS_COUNT 	2
// sensors
#define ULTRASONIC_COUNT	1
#define INFRARED_COUNT		1
#define BAMPER_COUNT		2

#define BAMPER_1 1
#define BAMPER_2 2

typedef struct Robot_2WD {
	uint8_t 	Bamper;
	int16_t 	PWM [MOTORS_COUNT];
	uint32_t 	US [ULTRASONIC_COUNT];
	uint32_t 	IR [INFRARED_COUNT];
	uint32_t 	Voltage;
} Robot_4WD;

uint16_t serialize_robot_2wd(Robot_2WD* src, uint8_t* dst, uint16_t dst_size);
uint16_t deserialize_robot_2wd(uint8_t* src, uint16_t src_size, Robot_2WD* dst);

#endif //#ifndef _ROBOT_2WD_H_
