#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#include "bsp_motor.h"
#include "questions.h"

#define Car_APB (188.0f)

void Motion_Car_Control(int16_t V_x, int16_t V_y, int16_t V_z);
void Motion_Yaw_Calc(float offset_yaw);
void Get_Odometry(void);

#endif
