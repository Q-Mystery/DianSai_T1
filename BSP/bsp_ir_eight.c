#include "bsp_ir_eight.h"
#include "app_control_config.h"
#include "delay.h"
#include "oled.h"

volatile uint8_t IR_Data_number[8] = {0};
uint8_t g_ir_white_level = EIGHT_IR_WHITE_LEVEL_DEFAULT;
uint8_t g_ir_black_level = EIGHT_IR_BLACK_LEVEL_DEFAULT;

#define IR_DISPLAY_VALUE_X      (24U)
#define IR_DISPLAY_VALUE_STEP   (12U)
#define IR_DISPLAY_TITLE_Y      (0U)
#define IR_DISPLAY_CH_Y         (8U)
#define IR_DISPLAY_LEVEL_Y      (16U)
#define IR_DISPLAY_NUM_Y        (24U)

void GPIO_setPins(GPIO_Regs *gpio, uint32_t pins, uint8_t value)
{
    if (value != 0U) {
        DL_GPIO_setPins(gpio, pins);
    } else {
        DL_GPIO_clearPins(gpio, pins);
    }
}

void ReadEightIR(volatile uint8_t ir_results[8])
{
    uint8_t channel;

    for (channel = 0U; channel < 8U; channel++) {
        uint8_t ad2 = (channel >> 2) & 0x01U;
        uint8_t ad1 = (channel >> 1) & 0x01U;
        uint8_t ad0 = channel & 0x01U;

        SET_CHANNEL(ad2, ad1, ad0);
        delay_us(EIGHT_IR_CHANNEL_SETTLE_US);
        ir_results[channel] = (READ_IR_OUT() != 0U) ? 1U : 0U;
    }
}

void EightIR_CalibrateWhite(uint8_t samples)
{
    uint16_t high_count = 0U;
    uint16_t total_count;
    uint8_t sample;
    uint8_t channel;

    if (samples == 0U) {
        samples = 1U;
    }

    total_count = (uint16_t)samples * 8U;
    for (sample = 0U; sample < samples; sample++) {
        ReadEightIR(IR_Data_number);
        for (channel = 0U; channel < 8U; channel++) {
            high_count += (IR_Data_number[channel] != 0U) ? 1U : 0U;
        }
    }

    /* Keep the complete sensor array over white during this calibration. */
    g_ir_white_level = (high_count * 2U >= total_count) ? 1U : 0U;
    g_ir_black_level = (uint8_t)!g_ir_white_level;
}

uint8_t EightIR_IsBlack(uint8_t logical_index)
{
    uint8_t raw_index;

    if (logical_index >= 8U) {
        return 0U;
    }

    raw_index = (EIGHT_IR_SENSOR_REVERSED != 0) ?
                    (uint8_t)(7U - logical_index) : logical_index;
    return (IR_Data_number[raw_index] == g_ir_black_level) ? 1U : 0U;
}

void OLED_SHOW_IR(void)
{
    uint8_t i;

    ReadEightIR(IR_Data_number);
    OLED_ShowString(0U, IR_DISPLAY_TITLE_Y, (uint8_t *)"Eight IR", 8U, 1U);
    OLED_ShowString(0U, IR_DISPLAY_CH_Y, (uint8_t *)"CH:", 8U, 1U);
    OLED_ShowString(0U, IR_DISPLAY_LEVEL_Y, (uint8_t *)"LV:", 8U, 1U);
    OLED_ShowString(0U, IR_DISPLAY_NUM_Y, (uint8_t *)"BK:", 8U, 1U);

    for (i = 0U; i < 8U; i++) {
        uint8_t x = (uint8_t)(IR_DISPLAY_VALUE_X + i * IR_DISPLAY_VALUE_STEP);
        OLED_ShowNum(x, IR_DISPLAY_CH_Y, i + 1U, 1U, 8U, 1U);
        OLED_ShowChar(x, IR_DISPLAY_LEVEL_Y,
                      (IR_Data_number[i] != 0U) ? 'H' : 'L', 8U, 1U);
        OLED_ShowNum(x, IR_DISPLAY_NUM_Y, EightIR_IsBlack(i), 1U, 8U, 1U);
    }
    OLED_Refresh();
}
