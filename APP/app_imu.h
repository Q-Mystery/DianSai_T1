#ifndef __APP_IMU_H__
#define __APP_IMU_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool available;
    bool is_curve;
    uint8_t who_am_i;
    int16_t gyro_z_raw;
    int16_t gyro_z_offset;
    int16_t yaw_rate_dps_x10;
    uint32_t last_update_ms;
} AppIMU_Status_t;

bool AppIMU_Init(void);
void AppIMU_Update(void);
const AppIMU_Status_t *AppIMU_GetStatus(void);

#endif
