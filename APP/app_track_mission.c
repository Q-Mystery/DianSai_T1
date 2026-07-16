#include "app_track_mission.h"
#include "AllHeader.h"
#include "app_imu.h"

static AppTrackMission_Status_t g_track_mission;
static int32_t s_mission_start_encoder[2];
static int32_t s_segment_start_encoder[2];
static uint32_t s_straight_counts;
static uint32_t s_arc_counts;
static uint32_t s_curve_gate_counts;
static uint32_t s_margin_counts;
static uint32_t s_total_stop_counts;
static uint32_t s_last_imu_update_ms;
static int32_t s_segment_yaw_abs_x10;

static uint32_t Mission_Abs32(int32_t value)
{
    return (value < 0) ? (uint32_t)(-value) : (uint32_t)value;
}

static uint32_t Mission_MmToCounts(uint32_t distance_mm)
{
    float counts = ((float)distance_mm * MD310_ENCODER_CIRCLE_PULSES) /
                   MECANUM_CIRCLE_MM;

    return (uint32_t)(counts + 0.5f);
}

static void Mission_ReadEncoder(int32_t encoder[2])
{
    int raw_encoder[2] = {0, 0};

    Encoder_Get_ALL(raw_encoder);
    encoder[0] = raw_encoder[0];
    encoder[1] = raw_encoder[1];
}

static uint32_t Mission_CountsFrom(const int32_t base_encoder[2])
{
    int32_t now_encoder[2];
    uint32_t left_delta;
    uint32_t right_delta;

    Mission_ReadEncoder(now_encoder);
    left_delta = Mission_Abs32(now_encoder[0] - base_encoder[0]);
    right_delta = Mission_Abs32(now_encoder[1] - base_encoder[1]);

    return (left_delta + right_delta) / 2U;
}

static void Mission_ResetSegment(void)
{
    Mission_ReadEncoder(s_segment_start_encoder);
    s_segment_yaw_abs_x10 = 0;
}

static void Mission_SetState(AppTrackMission_State_t state)
{
    g_track_mission.state = state;
    Mission_ResetSegment();
}

static void Mission_UpdateYaw(void)
{
    const AppIMU_Status_t *imu;
    uint32_t dt_ms;
    int32_t rate_x10;

    AppIMU_Update();
    imu = AppIMU_GetStatus();
    g_track_mission.imu_available = imu->available;

    if (!imu->available || (imu->last_update_ms == 0U)) {
        s_last_imu_update_ms = imu->last_update_ms;
        return;
    }

    if (s_last_imu_update_ms == 0U) {
        s_last_imu_update_ms = imu->last_update_ms;
        return;
    }

    if (imu->last_update_ms == s_last_imu_update_ms) {
        return;
    }

    dt_ms = (uint32_t)(imu->last_update_ms - s_last_imu_update_ms);
    s_last_imu_update_ms = imu->last_update_ms;
    rate_x10 = imu->yaw_rate_dps_x10;

    if (rate_x10 < 0) {
        rate_x10 = -rate_x10;
    }

    if (rate_x10 >= TRACK_MISSION_YAW_RATE_GATE_X10) {
        s_segment_yaw_abs_x10 += (int32_t)((rate_x10 * (int32_t)dt_ms) /
                                           1000);
    }
}

static bool Mission_StraightDone(uint32_t segment_counts)
{
    const AppIMU_Status_t *imu = AppIMU_GetStatus();

    if (segment_counts >= s_straight_counts) {
        return true;
    }

    return (imu->available && imu->is_curve &&
            (segment_counts >= s_curve_gate_counts));
}

static bool Mission_ArcDone(uint32_t segment_counts)
{
    const AppIMU_Status_t *imu = AppIMU_GetStatus();
    uint32_t near_arc_counts = (s_arc_counts > s_margin_counts) ?
                               (s_arc_counts - s_margin_counts) : 0U;

    if (!imu->available) {
        return segment_counts >= s_arc_counts;
    }

    if ((segment_counts >= near_arc_counts) &&
        (s_segment_yaw_abs_x10 >= TRACK_MISSION_ARC_YAW_CONFIRM_X10)) {
        return true;
    }

    return segment_counts >= (s_arc_counts + s_margin_counts);
}

void AppTrackMission_Init(void)
{
    uint32_t one_lap_mm;

    memset(&g_track_mission, 0, sizeof(g_track_mission));
    s_straight_counts = Mission_MmToCounts(TRACK_MISSION_STRAIGHT_MM);
    s_arc_counts = Mission_MmToCounts(TRACK_MISSION_ARC_LENGTH_MM);
    s_curve_gate_counts = Mission_MmToCounts(TRACK_MISSION_STRAIGHT_CURVE_GATE_MM);
    s_margin_counts = Mission_MmToCounts(TRACK_MISSION_DISTANCE_MARGIN_MM);
    one_lap_mm = (2U * TRACK_MISSION_STRAIGHT_MM) +
                 (2U * TRACK_MISSION_ARC_LENGTH_MM);
    s_total_stop_counts =
        Mission_MmToCounts(one_lap_mm * TRACK_MISSION_TARGET_LAPS);
    s_last_imu_update_ms = 0U;
    s_segment_yaw_abs_x10 = 0;
    Mission_ReadEncoder(s_mission_start_encoder);
    Mission_SetState(APP_TRACK_SEG_AB);
}

void AppTrackMission_Update(void)
{
    if (g_track_mission.state == APP_TRACK_STOPPED) {
        Motion_Stop(STOP_BRAKE);
        return;
    }

    Mission_UpdateYaw();
    LineWalking();

    g_track_mission.segment_counts = Mission_CountsFrom(s_segment_start_encoder);
    g_track_mission.total_counts = Mission_CountsFrom(s_mission_start_encoder);
    g_track_mission.segment_yaw_abs_x10 = s_segment_yaw_abs_x10;

    if (g_track_mission.total_counts >= s_total_stop_counts) {
        g_track_mission.lap_count = TRACK_MISSION_TARGET_LAPS;
        g_track_mission.state = APP_TRACK_STOPPED;
        Motion_Stop(STOP_BRAKE);
        return;
    }

    switch (g_track_mission.state) {
    case APP_TRACK_SEG_AB:
        if (Mission_StraightDone(g_track_mission.segment_counts)) {
            Mission_SetState(APP_TRACK_SEG_BC);
        }
        break;

    case APP_TRACK_SEG_BC:
        if (Mission_ArcDone(g_track_mission.segment_counts)) {
            Mission_SetState(APP_TRACK_SEG_CD);
        }
        break;

    case APP_TRACK_SEG_CD:
        if (Mission_StraightDone(g_track_mission.segment_counts)) {
            Mission_SetState(APP_TRACK_SEG_DA);
        }
        break;

    case APP_TRACK_SEG_DA:
        if (Mission_ArcDone(g_track_mission.segment_counts)) {
            g_track_mission.lap_count++;
            Mission_SetState(APP_TRACK_SEG_AB);
        }
        break;

    case APP_TRACK_STOPPED:
    default:
        g_track_mission.state = APP_TRACK_STOPPED;
        Motion_Stop(STOP_BRAKE);
        break;
    }
}

bool AppTrackMission_IsStopped(void)
{
    return g_track_mission.state == APP_TRACK_STOPPED;
}

const AppTrackMission_Status_t *AppTrackMission_GetStatus(void)
{
    return &g_track_mission;
}
