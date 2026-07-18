#include "app_gyro_straight.h"
#include "app_control_config.h"
#include "app_imu.h"

static AppGyroStraight_Status_t g_gyro_straight;

static int32_t GyroStraight_Abs32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static int32_t GyroStraight_Clamp32(int32_t value, int32_t limit)
{
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return -limit;
    }
    return value;
}

static int16_t GyroStraight_ClampCorrection(float value)
{
    if (value > (float)GYRO_STRAIGHT_MAX_DELTA_MM_S) {
        return GYRO_STRAIGHT_MAX_DELTA_MM_S;
    }
    if (value < (float)-GYRO_STRAIGHT_MAX_DELTA_MM_S) {
        return (int16_t)-GYRO_STRAIGHT_MAX_DELTA_MM_S;
    }
    return (int16_t)value;
}

void AppGyroStraight_Init(bool imu_available)
{
    AppGyroStraight_Reset();
    g_gyro_straight.enabled =
        (GYRO_STRAIGHT_ENABLE != 0U) && imu_available;
    g_gyro_straight.imu_available = imu_available;
}

void AppGyroStraight_Reset(void)
{
    g_gyro_straight.yaw_error_x10 = 0;
    g_gyro_straight.yaw_rate_x10 = 0;
    g_gyro_straight.correction_mm_s = 0;
    g_gyro_straight.last_update_ms = 0U;
}

int16_t AppGyroStraight_UpdateCorrection(void)
{
    const AppIMU_Status_t *imu;
    uint32_t dt_ms;
    int32_t yaw_delta_x10;
    int16_t yaw_rate_x10;
    float correction;

    if (!g_gyro_straight.enabled) {
        g_gyro_straight.correction_mm_s = 0;
        return 0;
    }

    AppIMU_Update();
    imu = AppIMU_GetStatus();
    g_gyro_straight.imu_available = imu->available;

    if (!imu->available || (imu->last_update_ms == 0U)) {
        g_gyro_straight.correction_mm_s = 0;
        return 0;
    }

    if (g_gyro_straight.last_update_ms == 0U) {
        g_gyro_straight.last_update_ms = imu->last_update_ms;
        g_gyro_straight.yaw_rate_x10 = imu->yaw_rate_dps_x10;
        g_gyro_straight.correction_mm_s = 0;
        return 0;
    }

    if (imu->last_update_ms == g_gyro_straight.last_update_ms) {
        return g_gyro_straight.correction_mm_s;
    }

    dt_ms = (uint32_t)(imu->last_update_ms - g_gyro_straight.last_update_ms);
    g_gyro_straight.last_update_ms = imu->last_update_ms;

    yaw_rate_x10 = imu->yaw_rate_dps_x10;
    if (GyroStraight_Abs32(yaw_rate_x10) <
        GYRO_STRAIGHT_RATE_DEADBAND_X10) {
        yaw_rate_x10 = 0;
    }
    g_gyro_straight.yaw_rate_x10 = yaw_rate_x10;

    yaw_delta_x10 = ((int32_t)yaw_rate_x10 * (int32_t)dt_ms) / 1000;
    g_gyro_straight.yaw_error_x10 = GyroStraight_Clamp32(
        g_gyro_straight.yaw_error_x10 + yaw_delta_x10,
        GYRO_STRAIGHT_YAW_LIMIT_X10);

    correction =
        ((float)g_gyro_straight.yaw_error_x10 / 10.0f) *
            GYRO_STRAIGHT_KP_MM_S_PER_DEG +
        ((float)yaw_rate_x10 / 10.0f) *
            GYRO_STRAIGHT_KD_MM_S_PER_DPS;
    correction *= (float)GYRO_STRAIGHT_CORRECTION_SIGN;

    g_gyro_straight.correction_mm_s =
        GyroStraight_ClampCorrection(correction);
    return g_gyro_straight.correction_mm_s;
}

const AppGyroStraight_Status_t *AppGyroStraight_GetStatus(void)
{
    return &g_gyro_straight;
}
