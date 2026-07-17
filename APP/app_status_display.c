#include "app_status_display.h"
#include "AllHeader.h"
#include "app_track_mission.h"
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

static uint8_t AppDisplay_ShowUnsigned(uint8_t x, uint8_t y,
    uint32_t value, uint8_t max_digits)
{
    uint32_t divisor = 1U;
    uint8_t i;
    uint8_t started = 0U;

    if (max_digits == 0U) {
        return x;
    }

    value = AppDisplay_ClampDigits(value, max_digits);
    for (i = 1U; i < max_digits; i++) {
        divisor *= 10U;
    }

    for (i = 0U; i < max_digits; i++) {
        uint8_t digit = (uint8_t)(value / divisor);
        value %= divisor;
        if ((digit != 0U) || (started != 0U) ||
            (i == (uint8_t)(max_digits - 1U))) {
            OLED_ShowChar(x, y, (uint8_t)('0' + digit), 8U, 1U);
            x = (uint8_t)(x + 6U);
            started = 1U;
        }
        divisor /= 10U;
    }

    return x;
}

static uint8_t AppDisplay_ShowFixed1(uint8_t x, uint8_t y,
    uint32_t value_x10, uint8_t integer_digits)
{
    uint32_t max_value_x10 = (AppDisplay_MaxForDigits(integer_digits) * 10U)
        + 9U;
    uint32_t integer;
    uint8_t decimal;

    if (value_x10 > max_value_x10) {
        value_x10 = max_value_x10;
    }

    integer = value_x10 / 10U;
    decimal = (uint8_t)(value_x10 % 10U);
    x = AppDisplay_ShowUnsigned(x, y, integer, integer_digits);
    OLED_ShowChar(x, y, '.', 8U, 1U);
    x = (uint8_t)(x + 6U);
    OLED_ShowChar(x, y, (uint8_t)('0' + decimal), 8U, 1U);
    x = (uint8_t)(x + 6U);

    return x;
}

static uint32_t AppDisplay_CountsToCm(uint32_t counts)
{
    float distance_cm = ((float)counts * MECANUM_CIRCLE_MM) /
        (MD310_ENCODER_CIRCLE_PULSES * 10.0f);

    return (uint32_t)(distance_cm + 0.5f);
}

static void AppDisplay_ClearRow(uint8_t row)
{
    OLED_ShowString(0U, (uint8_t)(row * 8U),
        (uint8_t *)"                     ", 8U, 1U);
}

void AppStatusDisplay_Update(void)
{
    int *encoder_counts = (int *)Motion_Get_Data(1U);
    uint32_t left_counts = AppDisplay_Abs32(encoder_counts[0]);
    uint32_t right_counts = AppDisplay_Abs32(encoder_counts[1]);
    uint32_t avg_counts = (left_counts + right_counts) / 2U;
    uint32_t travel_cm = AppDisplay_CountsToCm(avg_counts);
    float wheel_speed[2] = {0.0f, 0.0f};
    int16_t avg_speed_mm_s;
    uint32_t avg_speed_tenths_cm_s;
    uint8_t next_x;
    const AppTrackMission_Status_t *mission = AppTrackMission_GetStatus();
    const AppUltrasonic_Status_t *ultrasonic = AppUltrasonic_GetStatus();

    Motion_Get_Motor_Speed(wheel_speed);
    avg_speed_mm_s = (int16_t)((wheel_speed[0] + wheel_speed[1]) * 0.5f);
    if (avg_speed_mm_s < 0) {
        avg_speed_mm_s = (int16_t)(-avg_speed_mm_s);
    }
    avg_speed_tenths_cm_s = (uint32_t)avg_speed_mm_s;

    AppDisplay_ClearRow(0U);
    OLED_ShowString(0U, 0U, (uint8_t *)"T:", 8U, 1U);
    OLED_ShowNum(12U, 0U, Timer_Get_Runtime_Seconds99(), 2U, 8U, 1U);
    OLED_ShowString(24U, 0U, (uint8_t *)"s V:", 8U, 1U);
    next_x = AppDisplay_ShowFixed1(48U, 0U, avg_speed_tenths_cm_s, 3U);
    OLED_ShowString(next_x, 0U, (uint8_t *)"cm/s", 8U, 1U);

    AppDisplay_ClearRow(1U);
    OLED_ShowString(0U, 8U, (uint8_t *)"D:", 8U, 1U);
    next_x = AppDisplay_ShowUnsigned(12U, 8U, travel_cm, 5U);
    OLED_ShowString(next_x, 8U, (uint8_t *)"cm", 8U, 1U);
    OLED_ShowString(78U, 8U, (uint8_t *)"ARC:", 8U, 1U);
    OLED_ShowNum(102U, 8U, mission->arc_count, 1U, 8U, 1U);

    AppDisplay_ClearRow(2U);
    if (ultrasonic->obstacle) {
        OLED_ShowString(0U, 16U, (uint8_t *)"OBS:Y", 8U, 1U);
    } else {
        OLED_ShowString(0U, 16U, (uint8_t *)"OBS:N", 8U, 1U);
    }
    OLED_ShowString(36U, 16U, (uint8_t *)"U:", 8U, 1U);
    if (ultrasonic->valid) {
        next_x = AppDisplay_ShowUnsigned(48U, 16U,
            ultrasonic->distance_cm, 3U);
        OLED_ShowString(next_x, 16U, (uint8_t *)"cm", 8U, 1U);
    } else {
        OLED_ShowString(48U, 16U, (uint8_t *)"---cm", 8U, 1U);
    }
    OLED_ShowString(90U, 16U, (uint8_t *)"VE:", 8U, 1U);
    OLED_ShowNum(108U, 16U, AppVoice_GetLastError(), 1U, 8U, 1U);

    AppDisplay_ClearRow(3U);
    OLED_ShowString(0U, 24U, (uint8_t *)"ARC:", 8U, 1U);
    OLED_ShowNum(24U, 24U, mission->arc_count, 1U, 8U, 1U);
    OLED_ShowString(36U, 24U, (uint8_t *)"A:", 8U, 1U);
    next_x = AppDisplay_ShowUnsigned(48U, 24U,
        (AppDisplay_Abs32(mission->arc_angle_x10) + 5U) / 10U, 3U);
    OLED_ShowString(next_x, 24U, (uint8_t *)" S:", 8U, 1U);
    OLED_ShowNum((uint8_t)(next_x + 18U), 24U,
        (uint32_t)mission->state, 1U, 8U, 1U);

    OLED_Refresh();
}
