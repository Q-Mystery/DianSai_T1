#ifndef __APP_TRACK_MISSION_H__
#define __APP_TRACK_MISSION_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_TRACK_SEG_AB = 0,
    APP_TRACK_SEG_BC,
    APP_TRACK_SEG_CD,
    APP_TRACK_SEG_DA,
    APP_TRACK_STOPPED
} AppTrackMission_State_t;

typedef struct {
    AppTrackMission_State_t state;
    uint8_t lap_count;
    uint32_t segment_counts;
    uint32_t total_counts;
    int32_t segment_yaw_abs_x10;
    bool imu_available;
} AppTrackMission_Status_t;

void AppTrackMission_Init(void);
void AppTrackMission_Update(void);
bool AppTrackMission_IsStopped(void);
const AppTrackMission_Status_t *AppTrackMission_GetStatus(void);

#endif
