#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <stdint.h>

enum
{
	M_BREAK = 0,
	M_FORWARD,
	M_REVERSE
}motor_mode;

enum
{
	CLOSED_LOOP = 0,
	OPEN_LOOP
}motor_ctrlmthd;

typedef struct
{
	int spd;
	int cnt;
	uint8_t ctrlmthd;

	float kp;
	float ki;
	float kd;

	int error;
	int p_error;
	int inte;
	int diff;
	int pwmout;
}motor_typedef;

typedef struct
{
	uint8_t mode;
	int spd;
	int dirc;
}drive_typedef;

#endif
