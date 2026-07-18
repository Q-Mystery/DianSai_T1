#ifndef __BSP_MOTOR_H_
#define __BSP_MOTOR_H_

#include "ti_msp_dl_config.h"

#include <stdint.h>

/* TIMA0 uses a 1000-count EDGE_ALIGN_UP period in empty.syscfg. */
#define MOTOR_PWM_MAX                  (1000)
#define MOTOR_WHEEL_DIAMETER_MM        (67.0f)
#define MOTOR_ENCODER_COUNTS_PER_REV   (1760.0f)

void Motor_Init(void);
void Motor_Stop(void);
void Motor_SetSpeed(int16_t leftSpeed, int16_t rightSpeed);
void Motor_SetPWM(int16_t leftPWM, int16_t rightPWM);
void Motor_Control_1ms(void);

void Motor_GetEncoderCounts(int32_t *leftCount, int32_t *rightCount);
void Motor_GetMeasuredSpeeds(float *leftSpeed, float *rightSpeed);

#endif
