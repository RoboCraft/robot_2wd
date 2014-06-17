// 
// Base struct for 2WD robot
//
//
// robocraft.ru
//

#include "robot_2wd.h"

uint16_t serialize_robot_2wd(Robot_2WD* src, uint8_t* dst, uint16_t dst_size)
{
	if(!src || !dst || dst_size == 0)
		return -1;

	uint16_t l = 0;	
	dst[l++] = src->Bamper;

	int i;
	for(i=0; i<MOTORS_COUNT; i++) {
		orcp2::copy_int16(dst+l, src->PWM[i]);
		l += sizeof(src->PWM[i]);
	}
	for(i=0; i<ULTRASONIC_COUNT; i++) {
		orcp2::copy_int32(dst+l, src->US[i]);
		l += sizeof(src->US[i]);
	}
	for(i=0; i<INFRARED_COUNT; i++) {
		orcp2::copy_int32(dst+l, src->IR[i]);
		l += sizeof(src->IR[i]);
	}
	orcp2::copy_int32(dst+l, src->Voltage);
	l += sizeof(src->Voltage);

	return l;
}

uint16_t deserialize_robot_2wd(uint8_t* src, uint16_t src_size, Robot_2WD* dst)
{
	if(!src || src_size == 0 || !dst)
		return -1;

	uint16_t l = 0;
	dst->Bamper = src[l++];

	int i;
	for(i=0; i<MOTORS_COUNT; i++) {
		orcp2::copy_int16( (uint8_t *)&(dst->PWM[i]), *(uint16_t*)(src+l));
		l += sizeof(dst->PWM[i]);
	}
	for(i=0; i<ULTRASONIC_COUNT; i++) {
		orcp2::copy_int32( (uint8_t *)&(dst->US[i]), *(uint32_t*)(src+l));
		l += sizeof(dst->US[i]);
	}
	for(i=0; i<INFRARED_COUNT; i++) {
		orcp2::copy_int32( (uint8_t *)&(dst->IR[i]), *(uint32_t*)(src+l));
		l += sizeof(dst->IR[i]);
	}
	orcp2::copy_int32( (uint8_t *)&(dst->Voltage), *(uint32_t*)(src+l));
	l += sizeof(dst->Voltage);

	return l;
}
