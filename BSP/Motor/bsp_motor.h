#ifndef __BSP_MOTOR_H_
#define __BSP_MOTOR_H_

#include "AllHeader.h"

typedef enum
{
    MOTOR_ID_M1 = 0,
    MOTOR_ID_M2,
    MAX_MOTOR
} Motor_ID;

int myabs(int value);
int16_t speed_limit(int16_t speed, int16_t minimum, int16_t maximum);
void Init_Motor_PWM(void);
void PWM_Control_Car(int16_t left_motor_speed, int16_t right_motor_speed);
void L1_control(uint16_t motor_speed, uint8_t dir);
void R1_control(uint16_t motor_speed, uint8_t dir);
void Motor_Stop(uint8_t brake);
void Motor_Brake_Left(void);
void Motor_Brake_Right(void);

void Motor_Run(float left_speed, float right_speed);
void Motor_Right(float left_speed, float right_speed);
void Motor_Back(float left_speed, float right_speed);
void Motor_Left(float left_speed, float right_speed);

#endif
