#include "AllHeader.h"
#include "app_bcd_display.h"
#include "app_control_config.h"
#include "app_gyro_straight.h"
#include "app_imu.h"
#include "app_voice.h"

static bool g_line_stop_locked = false;
static uint8_t g_black_confirm_cycles = 0U;
static bool g_alarm_flash_on = false;
static uint32_t g_last_alarm_flash_ms = 0U;

static void LineStop_SetAlarmSignal(bool on)
{
    uint32_t pins = ULTRASONIC_SIGNAL_PB19_PIN | ULTRASONIC_SIGNAL_PB24_PIN;

    if (on) {
        DL_GPIO_setPins(ULTRASONIC_SIGNAL_PORT, pins);
    } else {
        DL_GPIO_clearPins(ULTRASONIC_SIGNAL_PORT, pins);
    }
}

static void LineStop_AlarmInit(void)
{
    uint32_t pins = ULTRASONIC_SIGNAL_PB19_PIN | ULTRASONIC_SIGNAL_PB24_PIN;

    DL_GPIO_initDigitalOutput(ULTRASONIC_SIGNAL_PB19_IOMUX);
    DL_GPIO_initDigitalOutput(ULTRASONIC_SIGNAL_PB24_IOMUX);
    LineStop_SetAlarmSignal(false);
    DL_GPIO_enableOutput(ULTRASONIC_SIGNAL_PORT, pins);
    Beep_OFF();
    CLOSE_MCULED();
}

static bool LineStop_IsBlackLineDetected(void)
{
    ReadEightIR(IR_Data_number);

    for (uint8_t i = 0U; i < 8U; i++) {
        if (EightIR_IsBlack(i) != 0U) {
            return true;
        }
    }

    return false;
}

static void LineStop_StartLockAlarm(void)
{
    g_line_stop_locked = true;
    g_alarm_flash_on = true;
    g_last_alarm_flash_ms = Timer_Get_Runtime_Ms();

    Motion_Emergency_Lock();
    LineStop_SetAlarmSignal(true);
    OPEN_MCULED();
    Beep_ON();
    AppBCDDisplay_WriteDigit(LINE_STOP_ALARM_BCD_CODE);
    (void)AppVoice_TriggerObstacle();
}

static void LineStop_UpdateAlarm(void)
{
    uint32_t now_ms = Timer_Get_Runtime_Ms();

    Motion_Emergency_Lock();
    Beep_ON();
    (void)AppVoice_TriggerObstacle();

    if ((uint32_t)(now_ms - g_last_alarm_flash_ms) >=
        LINE_STOP_ALARM_TOGGLE_MS) {
        g_last_alarm_flash_ms = now_ms;
        g_alarm_flash_on = !g_alarm_flash_on;
        LineStop_SetAlarmSignal(g_alarm_flash_on);
        if (g_alarm_flash_on) {
            OPEN_MCULED();
        } else {
            CLOSE_MCULED();
        }
    }
}

static void LineStop_UpdateDrive(void)
{
    int16_t gyro_correction;
    int16_t left_speed;
    int16_t right_speed;

    if (LineStop_IsBlackLineDetected()) {
        if (g_black_confirm_cycles < LINE_STOP_BLACK_CONFIRM_CYCLES) {
            g_black_confirm_cycles++;
        }
    } else {
        g_black_confirm_cycles = 0U;
    }

    if (g_black_confirm_cycles >= LINE_STOP_BLACK_CONFIRM_CYCLES) {
        LineStop_StartLockAlarm();
        return;
    }

    gyro_correction = AppGyroStraight_UpdateCorrection();
    left_speed = (int16_t)(LINE_STOP_STRAIGHT_SPEED_MM_S + gyro_correction);
    right_speed = (int16_t)(LINE_STOP_STRAIGHT_SPEED_MM_S - gyro_correction);
    Motion_Set_Speed(left_speed, right_speed);
}

int main(void)
{
    SYSCFG_DL_init();
    AppBCDDisplay_Init();
    OLED_Init();
    bool imu_ready = AppIMU_Init();
    AppVoice_Init();
    LineStop_AlarmInit();
    Init_Motor_PWM();
    Motor_Stop(STOP_FREE);

    /* Initialize PID state before the encoder timer can call Motion_Handle(). */
    PID_Param_Init();
    PID_Set_Motor_Parm(0U, MOTOR_SPEED_PID_KP, MOTOR_SPEED_PID_KI,
                       MOTOR_SPEED_PID_KD);
    PID_Set_Motor_Parm(1U, MOTOR_SPEED_PID_KP, MOTOR_SPEED_PID_KI,
                       MOTOR_SPEED_PID_KD);
    encoder_init();
    AppGyroStraight_Init(imu_ready);

#if LINE_STOP_CALIBRATE_WHITE_ON_BOOT
    delay_ms(100U);
    EightIR_CalibrateWhite(LINE_STOP_CALIBRATE_WHITE_SAMPLES);
#endif

    while (1) {
        AppBCDDisplay_Update();
        if (g_line_stop_locked) {
            LineStop_UpdateAlarm();
        } else {
            LineStop_UpdateDrive();
        }
        delay_ms(APP_MAIN_LOOP_DELAY_MS);
    }
}
