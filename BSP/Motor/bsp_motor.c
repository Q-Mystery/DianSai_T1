#include "bsp_motor.h"

#include <stdbool.h>

#define MOTOR_CONTROL_PERIOD_MS     (10U)
#define MOTOR_SPEED_KP              (0.35f)
#define MOTOR_SPEED_KI              (0.80f)
#define MOTOR_SPEED_FF              (1.00f)
#define PI_F                        (3.14159265358979323846f)

static volatile int32_t g_leftEncoderCount  = 0;
static volatile int32_t g_rightEncoderCount = 0;
static volatile int16_t g_leftTargetSpeed   = 0;
static volatile int16_t g_rightTargetSpeed  = 0;
static volatile float g_leftMeasuredSpeed   = 0.0f;
static volatile float g_rightMeasuredSpeed  = 0.0f;
static volatile bool g_speedControlEnabled  = false;

static uint8_t g_leftEncoderState  = 0;
static uint8_t g_rightEncoderState = 0;
static float g_leftIntegral        = 0.0f;
static float g_rightIntegral       = 0.0f;

static const int8_t g_quadratureTable[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

static int16_t clamp_motor_value(int32_t value)
{
    if (value > MOTOR_PWM_MAX) {
        return MOTOR_PWM_MAX;
    }
    if (value < -MOTOR_PWM_MAX) {
        return -MOTOR_PWM_MAX;
    }
    return (int16_t) value;
}

static uint8_t read_left_encoder_state(void)
{
    uint32_t pins = DL_GPIO_readPins(
        ENCODER_PORT, ENCODER_E1A_PIN | ENCODER_E1B_PIN);
    return (uint8_t) (((pins & ENCODER_E1A_PIN) ? 2U : 0U) |
                      ((pins & ENCODER_E1B_PIN) ? 1U : 0U));
}

static uint8_t read_right_encoder_state(void)
{
    uint32_t pins = DL_GPIO_readPins(
        ENCODER_PORT, ENCODER_E4A_PIN | ENCODER_E4B_PIN);
    return (uint8_t) (((pins & ENCODER_E4A_PIN) ? 2U : 0U) |
                      ((pins & ENCODER_E4B_PIN) ? 1U : 0U));
}

static void set_h_bridge(GPIO_Regs *port1, uint32_t pin1,
                         GPIO_Regs *port2, uint32_t pin2, int16_t command)
{
    if (command > 0) {
        DL_GPIO_setPins(port1, pin1);
        DL_GPIO_clearPins(port2, pin2);
    } else if (command < 0) {
        DL_GPIO_clearPins(port1, pin1);
        DL_GPIO_setPins(port2, pin2);
    } else {
        DL_GPIO_clearPins(port1, pin1);
        DL_GPIO_clearPins(port2, pin2);
    }
}

static void apply_motor_pwm(int16_t leftPWM, int16_t rightPWM)
{
    uint16_t leftDuty;
    uint16_t rightDuty;

    leftPWM  = clamp_motor_value(leftPWM);
    rightPWM = clamp_motor_value(rightPWM);
    leftDuty  = (uint16_t) ((leftPWM < 0) ? -leftPWM : leftPWM);
    rightDuty = (uint16_t) ((rightPWM < 0) ? -rightPWM : rightPWM);

    set_h_bridge(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN,
                 MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, leftPWM);
    set_h_bridge(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN,
                 MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, rightPWM);

    DL_TimerA_setCaptureCompareValue(
        BUZZER_INST, leftDuty, GPIO_BUZZER_C1_IDX);
    DL_TimerA_setCaptureCompareValue(
        BUZZER_INST, rightDuty, GPIO_BUZZER_C2_IDX);
}

static void update_encoder_counts(void)
{
    uint8_t leftState  = read_left_encoder_state();
    uint8_t rightState = read_right_encoder_state();

    g_leftEncoderCount +=
        g_quadratureTable[(g_leftEncoderState << 2) | leftState];
    g_rightEncoderCount +=
        g_quadratureTable[(g_rightEncoderState << 2) | rightState];

    g_leftEncoderState  = leftState;
    g_rightEncoderState = rightState;
}

void Motor_Init(void)
{
    g_leftEncoderState  = read_left_encoder_state();
    g_rightEncoderState = read_right_encoder_state();

    DL_GPIO_clearInterruptStatus(ENCODER_PORT,
        ENCODER_E1A_PIN | ENCODER_E1B_PIN |
        ENCODER_E4A_PIN | ENCODER_E4B_PIN);
    NVIC_ClearPendingIRQ(ENCODER_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_INT_IRQN);

    apply_motor_pwm(0, 0);
    DL_TimerA_startCounter(BUZZER_INST);
}

void Motor_Stop(void)
{
    Motor_SetPWM(0, 0);
}

void Motor_SetSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
    g_leftTargetSpeed  = clamp_motor_value(leftSpeed);
    g_rightTargetSpeed = clamp_motor_value(rightSpeed);
    g_speedControlEnabled = true;
}

void Motor_SetPWM(int16_t leftPWM, int16_t rightPWM)
{
    g_speedControlEnabled = false;
    g_leftTargetSpeed  = 0;
    g_rightTargetSpeed = 0;
    g_leftIntegral     = 0.0f;
    g_rightIntegral    = 0.0f;
    apply_motor_pwm(leftPWM, rightPWM);
}

void Motor_GetEncoderCounts(int32_t *leftCount, int32_t *rightCount)
{
    if (leftCount != 0) {
        *leftCount = g_leftEncoderCount;
    }
    if (rightCount != 0) {
        *rightCount = g_rightEncoderCount;
    }
}

void Motor_GetMeasuredSpeeds(float *leftSpeed, float *rightSpeed)
{
    if (leftSpeed != 0) {
        *leftSpeed = g_leftMeasuredSpeed;
    }
    if (rightSpeed != 0) {
        *rightSpeed = g_rightMeasuredSpeed;
    }
}

void Motor_Control_1ms(void)
{
    static uint8_t divider = 0;
    static int32_t previousLeftCount  = 0;
    static int32_t previousRightCount = 0;
    const float millimetersPerCount =
        (PI_F * MOTOR_WHEEL_DIAMETER_MM) / MOTOR_ENCODER_COUNTS_PER_REV;
    int32_t leftCount;
    int32_t rightCount;
    int32_t leftDelta;
    int32_t rightDelta;
    float leftMeasured;
    float rightMeasured;
    float leftError;
    float rightError;
    int32_t leftOutput;
    int32_t rightOutput;

    divider++;
    if (divider < MOTOR_CONTROL_PERIOD_MS) {
        return;
    }
    divider = 0;

    Motor_GetEncoderCounts(&leftCount, &rightCount);
    leftDelta  = leftCount - previousLeftCount;
    rightDelta = rightCount - previousRightCount;
    previousLeftCount  = leftCount;
    previousRightCount = rightCount;

    if (leftDelta < 0) leftDelta = -leftDelta;
    if (rightDelta < 0) rightDelta = -rightDelta;

    leftMeasured = (float) leftDelta * millimetersPerCount * 100.0f;
    rightMeasured = (float) rightDelta * millimetersPerCount * 100.0f;
    if (g_leftTargetSpeed < 0) leftMeasured = -leftMeasured;
    if (g_rightTargetSpeed < 0) rightMeasured = -rightMeasured;
    g_leftMeasuredSpeed  = leftMeasured;
    g_rightMeasuredSpeed = rightMeasured;

    if (!g_speedControlEnabled) {
        return;
    }

    if (g_leftTargetSpeed == 0 && g_rightTargetSpeed == 0) {
        g_leftIntegral  = 0.0f;
        g_rightIntegral = 0.0f;
        apply_motor_pwm(0, 0);
        return;
    }

    leftError  = (float) g_leftTargetSpeed - leftMeasured;
    rightError = (float) g_rightTargetSpeed - rightMeasured;
    g_leftIntegral  += leftError * 0.01f;
    g_rightIntegral += rightError * 0.01f;

    if (g_leftIntegral > 600.0f) g_leftIntegral = 600.0f;
    if (g_leftIntegral < -600.0f) g_leftIntegral = -600.0f;
    if (g_rightIntegral > 600.0f) g_rightIntegral = 600.0f;
    if (g_rightIntegral < -600.0f) g_rightIntegral = -600.0f;

    leftOutput = (int32_t) (MOTOR_SPEED_FF * g_leftTargetSpeed +
        MOTOR_SPEED_KP * leftError + MOTOR_SPEED_KI * g_leftIntegral);
    rightOutput = (int32_t) (MOTOR_SPEED_FF * g_rightTargetSpeed +
        MOTOR_SPEED_KP * rightError + MOTOR_SPEED_KI * g_rightIntegral);
    apply_motor_pwm(
        clamp_motor_value(leftOutput), clamp_motor_value(rightOutput));
}

/* GPIOA and GPIOB share the GROUP1 vector on MSPM0G3507. */
void GROUP1_IRQHandler(void)
{
    const uint32_t encoderPins = ENCODER_E1A_PIN | ENCODER_E1B_PIN |
                                 ENCODER_E4A_PIN | ENCODER_E4B_PIN;
    uint32_t pending = DL_GPIO_getEnabledInterruptStatus(
        ENCODER_PORT, encoderPins);

    if (pending != 0U) {
        DL_GPIO_clearInterruptStatus(ENCODER_PORT, pending);
        update_encoder_counts();
    }
}
