#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

#include "AllHeader.h"

void Timer_20ms_Init(void);
void Timer_Display_Init(void);
uint32_t Timer_Get_Runtime_Ms(void);
uint8_t Timer_Get_Runtime_Seconds99(void);
#endif

