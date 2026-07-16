#ifndef __BSP_IR_EIGHT_H__
#define __BSP_IR_EIGHT_H__

#include "ti_msp_dl_config.h"

#define EIGHT_IR_PORT        Eight_IR_PORT
#define EIGHT_IR_AD0_PIN     Eight_IR_AD0_PIN
#define EIGHT_IR_AD1_PIN     Eight_IR_AD1_PIN
#define EIGHT_IR_AD2_PIN     Eight_IR_AD2_PIN
#define EIGHT_IR_OUT_PIN     Eight_IR_OUT_PIN

#define SET_CHANNEL(ad2, ad1, ad0)                             \
    do {                                                       \
        GPIO_setPins(EIGHT_IR_PORT, EIGHT_IR_AD2_PIN, (ad2));  \
        GPIO_setPins(EIGHT_IR_PORT, EIGHT_IR_AD1_PIN, (ad1));  \
        GPIO_setPins(EIGHT_IR_PORT, EIGHT_IR_AD0_PIN, (ad0));  \
    } while (0)

#define READ_IR_OUT()  DL_GPIO_readPins(EIGHT_IR_PORT, EIGHT_IR_OUT_PIN)

extern volatile uint8_t IR_Data_number[8];
extern uint8_t g_ir_white_level;
extern uint8_t g_ir_black_level;

void GPIO_setPins(GPIO_Regs *gpio, uint32_t pins, uint8_t value);
void ReadEightIR(volatile uint8_t ir_results[8]);
void EightIR_CalibrateWhite(uint8_t samples);
uint8_t EightIR_IsBlack(uint8_t logical_index);
void OLED_SHOW_IR(void);

#endif
