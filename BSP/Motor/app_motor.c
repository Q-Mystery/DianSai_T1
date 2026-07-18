#include "app_motor.h"

static int g_leftSpeedSetpoint  = 0;
static int g_rightSpeedSetpoint = 0;

static int clamp_speed(int speed)
{
    if (speed > MOTOR_PWM_MAX) {
        return MOTOR_PWM_MAX;
    }
    if (speed < -MOTOR_PWM_MAX) {
        return -MOTOR_PWM_MAX;
    }
    return speed;
}

/* Differential-drive kinematics for the two-wheel chassis. */
void Motion_Car_Control(int16_t V_x, int16_t V_y, int16_t V_z)
{
    float spinSpeed;

    (void) V_y;
    if (V_x == 0 && V_z == 0) {
        g_leftSpeedSetpoint  = 0;
        g_rightSpeedSetpoint = 0;
        Motor_Stop();
        return;
    }

    spinSpeed = ((float) V_z / 1000.0f) * Car_APB;
    g_leftSpeedSetpoint  = clamp_speed((int) ((float) V_x + spinSpeed));
    g_rightSpeedSetpoint = clamp_speed((int) ((float) V_x - spinSpeed));
    Motor_SetSpeed(g_leftSpeedSetpoint, g_rightSpeedSetpoint);
}

void Motion_Yaw_Calc(float offset_yaw)
{
    int leftSpeed = clamp_speed(
        g_leftSpeedSetpoint + (int) offset_yaw);
    int rightSpeed = clamp_speed(
        g_rightSpeedSetpoint - (int) offset_yaw);

    Motor_SetSpeed(leftSpeed, rightSpeed);
}

/* Accumulate distance from the left and right wheel encoders. */
void Get_Odometry(void)
{
    static int32_t previousLeftCount  = 0;
    static int32_t previousRightCount = 0;
    const float millimetersPerCount =
        (3.14159265358979323846f * MOTOR_WHEEL_DIAMETER_MM) /
        MOTOR_ENCODER_COUNTS_PER_REV;

    if (encoder_odometry_flag) {
        int32_t leftCount;
        int32_t rightCount;
        int32_t leftDelta;
        int32_t rightDelta;

        Motor_GetEncoderCounts(&leftCount, &rightCount);
        leftDelta  = leftCount - previousLeftCount;
        rightDelta = rightCount - previousRightCount;
        previousLeftCount  = leftCount;
        previousRightCount = rightCount;

        if (leftDelta < 0) leftDelta = -leftDelta;
        if (rightDelta < 0) rightDelta = -rightDelta;
        odometry_sum += (int) ((((float) leftDelta + (float) rightDelta) *
            0.5f * millimetersPerCount) + 0.5f);
    } else {
        Motor_GetEncoderCounts(
            &previousLeftCount, &previousRightCount);
    }
}
