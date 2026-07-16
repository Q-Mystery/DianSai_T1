#include "bsp_beep_led.h"


/**
 * @brief 打开MCU指示灯  Turn on MCU indicator LED
 */
void OPEN_MCULED(void)
{
#if BEEP_LED_GPIO_ENABLED
    DL_GPIO_setPins(LED_PORT, LED_MCU_PIN);  // 置位LED引脚  Set LED pin
#endif
}

/**
 * @brief 关闭MCU指示灯 / Turn off MCU indicator LED
 */
void CLOSE_MCULED(void)
{
#if BEEP_LED_GPIO_ENABLED
    DL_GPIO_clearPins(LED_PORT, LED_MCU_PIN);  // 清除LED引脚  Clear LED pin
#endif
}

/**
 * @brief 打开蜂鸣器  Turn on buzzer
 */

void Beep_ON(void)
{
#if BEEP_LED_GPIO_ENABLED
	DL_GPIO_setPins(BEEP_PORT,BEEP_OUT_PIN);
#endif
}

void Beep_OFF(void)
{
#if BEEP_LED_GPIO_ENABLED
	DL_GPIO_clearPins(BEEP_PORT,BEEP_OUT_PIN);
#endif
}
