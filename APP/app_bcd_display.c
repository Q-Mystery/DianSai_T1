#include "app_bcd_display.h"
#include "AllHeader.h"

#define BCD_DISPLAY_PIN_MASK \
    (BCD_DISPLAY_B0_PIN | BCD_DISPLAY_B1_PIN | \
     BCD_DISPLAY_B2_PIN | BCD_DISPLAY_B3_PIN)

static uint8_t s_last_digit = 0xFFU;

void AppBCDDisplay_WriteDigit(uint8_t digit)
{
    uint32_t pins = 0U;

    digit %= 10U;
    if ((digit & 0x01U) != 0U) {
        pins |= BCD_DISPLAY_B0_PIN;
    }
    if ((digit & 0x02U) != 0U) {
        pins |= BCD_DISPLAY_B1_PIN;
    }
    if ((digit & 0x04U) != 0U) {
        pins |= BCD_DISPLAY_B2_PIN;
    }
    if ((digit & 0x08U) != 0U) {
        pins |= BCD_DISPLAY_B3_PIN;
    }

    DL_GPIO_clearPins(BCD_DISPLAY_PORT, BCD_DISPLAY_PIN_MASK & ~pins);
    DL_GPIO_setPins(BCD_DISPLAY_PORT, pins);
    s_last_digit = digit;
}

void AppBCDDisplay_Init(void)
{
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B0_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B1_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B2_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B3_IOMUX);
    DL_GPIO_clearPins(BCD_DISPLAY_PORT, BCD_DISPLAY_PIN_MASK);
    DL_GPIO_enableOutput(BCD_DISPLAY_PORT, BCD_DISPLAY_PIN_MASK);
    AppBCDDisplay_WriteDigit(0U);
}

void AppBCDDisplay_Update(void)
{
    uint8_t digit = (uint8_t)((Timer_Get_Runtime_Ms() / 1000U) % 10U);

    if (digit != s_last_digit) {
        AppBCDDisplay_WriteDigit(digit);
    }
}
