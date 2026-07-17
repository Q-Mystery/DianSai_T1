#ifndef __APP_TRACK_MISSION_H__
#define __APP_TRACK_MISSION_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_TRACK_WAIT_CURVE = 0,
    APP_TRACK_ARC_TRACKING,
    APP_TRACK_ARC_WAIT_EXIT,
    APP_TRACK_COOLDOWN,
    APP_TRACK_STOPPED
} AppTrackMission_State_t;

typedef struct {
    AppTrackMission_State_t state;
    uint8_t arc_count;
    bool imu_available;
    int8_t arc_direction;
    int32_t yaw_rate_filtered_x10;
    int32_t arc_angle_x10;
    uint32_t state_elapsed_ms;
} AppTrackMission_Status_t;

void AppTrackMission_Init(void);
void AppTrackMission_Update(void);
bool AppTrackMission_IsStopped(void);
const AppTrackMission_Status_t *AppTrackMission_GetStatus(void);

#endif
