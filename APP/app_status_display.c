#include "app_status_display.h"
#include "AllHeader.h"
#include "app_imu.h"
#include "app_ultrasonic.h"
#include "app_voice.h"

static uint32_t AppDisplay_Abs32(int32_t value)
{
    return (value < 0) ? (uint32_t)(-value) : (uint32_t)value;
}

static uint32_t AppDisplay_MaxForDigits(uint8_t digits)
{
    uint8_t i;
    uint32_t max_value = 0U;

    for (i = 0U; i < digits; i++) {
        max_value = (max_value * 10U) + 9U;
    }
    return max_value;
}

static uint32_t AppDisplay_ClampDigits(uint32_t value, uint8_t digits)
{
    uint32_t max_value = AppDisplay_MaxForDigits(digits);

    return (value > max_value) ? max_value : value;
}

static void AppDisplay_ClearRow(uint8_t row)
{
    OLED_ShowString(0U, (uint8_t)(row * 8U),
        (uint8_t *)"                     ", 8U, 1U);
}

static void AppDisplay_ShowSigned3(uint8_t x, uint8_t y, int16_t value)
{
    uint32_t abs_value;

    if (value < 0) {
        OLED_ShowChar(x, y, '-', 8U, 1U);
        abs_value = AppDisplay_Abs32(value);
    } else {
        OLED_ShowChar(x, y, '+', 8U, 1U);
        abs_value = (uint32_t)value;
    }

    OLED_ShowNum((uint8_t)(x + 6U), y,
        AppDisplay_ClampDigits(abs_value, 3U), 3U, 8U, 1U);
}

void AppStatusDisplay_Update(void)
{
    int *encoder_counts = (int *)Motion_Get_Data(1U);
    uint32_t left_counts = AppDisplay_Abs32(encoder_counts[0]);
    uint32_t right_counts = AppDisplay_Abs32(encoder_counts[1]);
    uint32_t avg_counts = (left_counts + right_counts) / 2U;
    float wheel_speed[2] = {0.0f, 0.0f};
    int16_t avg_speed;
    const AppIMU_Status_t *imu = AppIMU_GetStatus();
    const AppUltrasonic_Status_t *ultrasonic = AppUltrasonic_GetStatus();

    Motion_Get_Motor_Speed(wheel_speed);
    avg_speed = (int16_t)((wheel_speed[0] + wheel_speed[1]) * 0.5f);
    if (avg_speed < 0) {
        avg_speed = (int16_t)(-avg_speed);
    }

    AppDisplay_ClearRow(0U);
    OLED_ShowString(0U, 0U, (uint8_t *)"T:", 8U, 1U);
    OLED_ShowNum(12U, 0U, Timer_Get_Runtime_Seconds99(), 2U, 8U, 1U);
    OLED_ShowString(24U, 0U, (uint8_t *)"s V:", 8U, 1U);
    OLED_ShowNum(48U, 0U,
        AppDisplay_ClampDigits((uint32_t)avg_speed, 3U), 3U, 8U, 1U);

    AppDisplay_ClearRow(1U);
    OLED_ShowString(0U, 8U, (uint8_t *)"E:", 8U, 1U);
    OLED_ShowNum(12U, 8U,
        AppDisplay_ClampDigits(avg_counts, 6U), 6U, 8U, 1U);

    AppDisplay_ClearRow(2U);
    if (ultrasonic->obstacle) {
        OLED_ShowString(0U, 16U, (uint8_t *)"OBS:YES", 8U, 1U);
    } else {
        OLED_ShowString(0U, 16U, (uint8_t *)"OBS:NO", 8U, 1U);
    }
    OLED_ShowString(48U, 16U, (uint8_t *)"D:", 8U, 1U);
    if (ultrasonic->valid) {
        OLED_ShowNum(60U, 16U,
            AppDisplay_ClampDigits(ultrasonic->distance_cm, 3U), 3U, 8U, 1U);
    } else {
        OLED_ShowString(60U, 16U, (uint8_t *)"---", 8U, 1U);
    }
    OLED_ShowString(78U, 16U, (uint8_t *)"cm V:", 8U, 1U);
    OLED_ShowNum(108U, 16U, AppVoice_GetLastError(), 1U, 8U, 1U);

    AppDisplay_ClearRow(3U);
    if (imu->available) {
        OLED_ShowString(0U, 24U,
            (uint8_t *)(imu->is_curve ? "IMU:CUR Z:" : "IMU:STR Z:"),
            8U, 1U);
        AppDisplay_ShowSigned3(60U, 24U, imu->yaw_rate_dps_x10);
    } else {
        OLED_ShowString(0U, 24U, (uint8_t *)"IMU:MISS", 8U, 1U);
    }

    OLED_Refresh();
}
