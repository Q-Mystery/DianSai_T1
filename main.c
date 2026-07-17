#include "AllHeader.h"
#include "app_bcd_display.h"
#include "app_control_config.h"
#include "app_imu.h"
#include "app_status_display.h"
#include "app_track_mission.h"
#include "app_ultrasonic.h"
#include "app_voice.h"

int main(void)
{
    uint8_t display_divider = 0U;
    bool obstacle_now = false;
    bool obstacle_last = false;

    SYSCFG_DL_init();
    AppBCDDisplay_Init();
    OLED_Init();
    (void)AppIMU_Init();
    AppUltrasonic_Init();
    AppVoice_Init();
    Init_Motor_PWM();
    Motor_Stop(STOP_FREE);

    /* Initialize PID state before the encoder timer can call Motion_Handle(). */
    PID_Param_Init();
    PID_Set_Motor_Parm(0U, MOTOR_SPEED_PID_KP, MOTOR_SPEED_PID_KI,
                       MOTOR_SPEED_PID_KD);
    PID_Set_Motor_Parm(1U, MOTOR_SPEED_PID_KP, MOTOR_SPEED_PID_KI,
                       MOTOR_SPEED_PID_KD);
    encoder_init();
    AppTrackMission_Init();

    while (1) {
        AppBCDDisplay_Update();
        AppUltrasonic_Update();
        obstacle_now = AppUltrasonic_IsObstacle();
        if (obstacle_now) {
            Motion_Stop(STOP_BRAKE);
            (void)AppVoice_TriggerObstacle();
        } else {
            AppTrackMission_Update();
        }
        if (obstacle_now != obstacle_last) {
            display_divider = APP_OLED_DISPLAY_DIVIDER;
            obstacle_last = obstacle_now;
        }
        display_divider++;
        if (display_divider >= APP_OLED_DISPLAY_DIVIDER) {
            display_divider = 0U;
            AppIMU_Update();
            AppStatusDisplay_Update();
        }
        delay_ms(APP_MAIN_LOOP_DELAY_MS);
    }
}
