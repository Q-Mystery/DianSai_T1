#ifndef __APP_BCD_DISPLAY_H__
#define __APP_BCD_DISPLAY_H__

#include <stdint.h>

void AppBCDDisplay_Init(void);
void AppBCDDisplay_WriteDigit(uint8_t digit);
void AppBCDDisplay_Update(void);

#endif
