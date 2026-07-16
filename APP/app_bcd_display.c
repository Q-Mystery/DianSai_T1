#include "app_bcd_display.h"
#include "AllHeader.h"

#define BCD_DISPLAY_PIN_MASK \
    (BCD_DISPLAY_B0_PIN | BCD_DISPLAY_B1_PIN | \
     BCD_DISPLAY_B2_PIN | BCD_DISPLAY_B3_PIN)

#define BCD_DISPLAY_DIGIT_PIN_MASK \
    (BCD_DIGIT_SELECT_TENS_PIN | BCD_DIGIT_SELECT_ONES_PIN)

static uint8_t s_scan_digit = 0U;

static void AppBCDDisplay_BlankDigits(void)
{
#if BCD_DIGIT_SELECT_ACTIVE_HIGH
    DL_GPIO_clearPins(BCD_DIGIT_SELECT_PORT, BCD_DISPLAY_DIGIT_PIN_MASK);
#else
    DL_GPIO_setPins(BCD_DIGIT_SELECT_PORT, BCD_DISPLAY_DIGIT_PIN_MASK);
#endif
}

static void AppBCDDisplay_EnableDigit(uint32_t digit_pin)
{
#if BCD_DIGIT_SELECT_ACTIVE_HIGH
    DL_GPIO_setPins(BCD_DIGIT_SELECT_PORT, digit_pin);
#else
    DL_GPIO_clearPins(BCD_DIGIT_SELECT_PORT, digit_pin);
#endif
}

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
}

void AppBCDDisplay_Init(void)
{
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B0_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B1_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B2_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DISPLAY_B3_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DIGIT_SELECT_TENS_IOMUX);
    DL_GPIO_initDigitalOutput(BCD_DIGIT_SELECT_ONES_IOMUX);
    DL_GPIO_clearPins(BCD_DISPLAY_PORT, BCD_DISPLAY_PIN_MASK);
    AppBCDDisplay_BlankDigits();
    DL_GPIO_enableOutput(BCD_DISPLAY_PORT, BCD_DISPLAY_PIN_MASK);
    DL_GPIO_enableOutput(BCD_DIGIT_SELECT_PORT, BCD_DISPLAY_DIGIT_PIN_MASK);
    AppBCDDisplay_WriteDigit(0U);
}

void AppBCDDisplay_Update(void)
{
    uint8_t seconds = Timer_Get_Runtime_Seconds99();
    uint8_t digit;
    uint32_t digit_pin;

    if (s_scan_digit == 0U) {
        digit = (uint8_t)(seconds / 10U);
        digit_pin = BCD_DIGIT_SELECT_TENS_PIN;
        s_scan_digit = 1U;
    } else {
        digit = (uint8_t)(seconds % 10U);
        digit_pin = BCD_DIGIT_SELECT_ONES_PIN;
        s_scan_digit = 0U;
    }

    AppBCDDisplay_BlankDigits();
    AppBCDDisplay_WriteDigit(digit);
    AppBCDDisplay_EnableDigit(digit_pin);
}
