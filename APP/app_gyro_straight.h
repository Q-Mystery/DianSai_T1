#ifndef __APP_GYRO_STRAIGHT_H__
#define __APP_GYRO_STRAIGHT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool enabled;
    bool imu_available;
    int32_t yaw_error_x10;
    int16_t yaw_rate_x10;
    int16_t correction_mm_s;
    uint32_t last_update_ms;
} AppGyroStraight_Status_t;

void AppGyroStraight_Init(bool imu_available);
void AppGyroStraight_Reset(void);
int16_t AppGyroStraight_UpdateCorrection(void);
const AppGyroStraight_Status_t *AppGyroStraight_GetStatus(void);

#endif
