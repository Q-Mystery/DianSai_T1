#ifndef __BSP_BEEP_LED_H_
#define __BSP_BEEP_LED_H_

#include "AllHeader.h"

/* Line-following build leaves the optional LED/buzzer pins unassigned. */
#define BEEP_LED_GPIO_ENABLED (0)

void Beep_ON(void);
void Beep_OFF(void);
void OPEN_MCULED(void);
void CLOSE_MCULED(void);


#endif
