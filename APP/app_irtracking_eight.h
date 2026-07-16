#ifndef __APP_IRTRACKING_EIGHT_H__
#define __APP_IRTRACKING_EIGHT_H__

#include <stdint.h>

#define BLACK  (1)
#define WHITE  (0)

extern uint8_t X1, X2, X3, X4, X5, X6, X7, X8;
extern int pid_output_IRR;

float APP_HD_PID_Calc(int8_t actual_value);
void Copy_HD_Data(void);
int LineCheck(void);
void Line_Tracke(void);
void LineWalking_PWM(void);
void deal_IRdata(uint8_t *x1, uint8_t *x2, uint8_t *x3, uint8_t *x4,
                 uint8_t *x5, uint8_t *x6, uint8_t *x7, uint8_t *x8);
void LineWalking(void);

#endif
