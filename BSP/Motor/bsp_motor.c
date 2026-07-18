#include "bsp_motor.h"

static void write_gpio(GPIO_Regs *port, uint32_t pin, uint8_t value)
{
    if (value != 0U) {
        DL_GPIO_setPins(port, pin);
    } else {
        DL_GPIO_clearPins(port, pin);
    }
}

static uint16_t add_dead_zone(uint16_t pwm, uint16_t dead_zone)
{
    uint32_t adjusted;

    if (pwm == 0U) {
        return 0U;
    }

    adjusted = (uint32_t)pwm + dead_zone;
    return (adjusted > MOTOR_PWM_MAX_DUTY) ?
               MOTOR_PWM_MAX_DUTY : (uint16_t)adjusted;
}

static uint16_t logical_pwm_to_compare(uint16_t pwm)
{
    if (pwm > MOTOR_PWM_MAX_DUTY) {
        pwm = MOTOR_PWM_MAX_DUTY;
    }

    if (MOTOR_PWM_COMPARE_INVERTED != 0U) {
        return (uint16_t)(MOTOR_PWM_MAX_DUTY - pwm);
    }
    return pwm;
}

static void set_left_direction(uint8_t reverse)
{
    reverse = (uint8_t)((reverse != 0U) ^ LEFT_MOTOR_DIR_REVERSED);
    write_gpio(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN, (uint8_t)!reverse);
    write_gpio(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, reverse);
}

static void set_right_direction(uint8_t reverse)
{
    reverse = (uint8_t)((reverse != 0U) ^ RIGHT_MOTOR_DIR_REVERSED);
    write_gpio(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN, (uint8_t)!reverse);
    write_gpio(MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, reverse);
}

static void set_left_pwm(uint16_t pwm)
{
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST,
                                     logical_pwm_to_compare(pwm),
                                     MOTOR_LEFT_PWM_CHANNEL_INDEX);
}

static void set_right_pwm(uint16_t pwm)
{
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST,
                                     logical_pwm_to_compare(pwm),
                                     MOTOR_RIGHT_PWM_CHANNEL_INDEX);
}

void Init_Motor_PWM(void)
{
    Motor_Stop(0U);
    DL_TimerA_startCounter(motor_PWM_INST);
}

int16_t speed_limit(int16_t speed, int16_t minimum, int16_t maximum)
{
    if (speed < minimum) {
        return minimum;
    }
    if (speed > maximum) {
        return maximum;
    }
    return speed;
}

int myabs(int value)
{
    return (value < 0) ? -value : value;
}

void L1_control(uint16_t motor_speed, uint8_t dir)
{
    uint16_t pwm = (motor_speed > MOTOR_PWM_MAX_DUTY) ?
                       MOTOR_PWM_MAX_DUTY : motor_speed;

    /* Logical reverse is forbidden; chassis-forward pin polarity is fixed. */
    (void)dir;

    set_left_pwm(0U);
    if (pwm == 0U) {
        write_gpio(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN, 0U);
        write_gpio(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, 0U);
        return;
    }

    set_left_direction(0U);
    set_left_pwm(add_dead_zone(pwm, LEFT_MOTOR_PWM_DEAD_ZONE));
}

void R1_control(uint16_t motor_speed, uint8_t dir)
{
    uint16_t pwm = (motor_speed > MOTOR_PWM_MAX_DUTY) ?
                       MOTOR_PWM_MAX_DUTY : motor_speed;

    /* Logical reverse is forbidden; motor_D remains electrically mirrored. */
    (void)dir;

    set_right_pwm(0U);
    if (pwm == 0U) {
        write_gpio(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN, 0U);
        write_gpio(MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, 0U);
        return;
    }

    set_right_direction(0U);
    set_right_pwm(add_dead_zone(pwm, RIGHT_MOTOR_PWM_DEAD_ZONE));
}

void PWM_Control_Car(int16_t left_motor_speed, int16_t right_motor_speed)
{
    int16_t left = speed_limit(left_motor_speed, 0, MOTOR_PWM_MAX_DUTY);
    int16_t right = speed_limit(right_motor_speed, 0, MOTOR_PWM_MAX_DUTY);

    L1_control((uint16_t)left, 0U);
    R1_control((uint16_t)right, 0U);
}

void Motor_Brake_Left(void)
{
    write_gpio(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN, 1U);
    write_gpio(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, 1U);
    set_left_pwm(MOTOR_PWM_MAX_DUTY);
}

void Motor_Brake_Right(void)
{
    write_gpio(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN, 1U);
    write_gpio(MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, 1U);
    set_right_pwm(MOTOR_PWM_MAX_DUTY);
}

void Motor_Stop(uint8_t brake)
{
    if (brake != 0U) {
        write_gpio(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN, 1U);
        write_gpio(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, 1U);
        write_gpio(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN, 1U);
        write_gpio(MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, 1U);
        set_left_pwm(MOTOR_PWM_MAX_DUTY);
        set_right_pwm(MOTOR_PWM_MAX_DUTY);
    } else {
        set_left_pwm(0U);
        set_right_pwm(0U);
        write_gpio(MOTOR_DIR_AIN1_PORT, MOTOR_DIR_AIN1_PIN, 0U);
        write_gpio(MOTOR_DIR_AIN2_PORT, MOTOR_DIR_AIN2_PIN, 0U);
        write_gpio(MOTOR_DIR_DIN1_PORT, MOTOR_DIR_DIN1_PIN, 0U);
        write_gpio(MOTOR_DIR_DIN2_PORT, MOTOR_DIR_DIN2_PIN, 0U);
    }
}

void Motor_Run(float left_speed, float right_speed)
{
    L1_control((uint16_t)left_speed, 0U);
    R1_control((uint16_t)right_speed, 0U);
}

void Motor_Back(float left_speed, float right_speed)
{
    (void)left_speed;
    (void)right_speed;
    Motor_Stop(0U);
}

void Motor_Left(float left_speed, float right_speed)
{
    L1_control((uint16_t)(left_speed * 0.5f), 0U);
    R1_control((uint16_t)right_speed, 0U);
    delay_ms(500U);
    Motor_Stop(1U);
}

void Motor_Right(float left_speed, float right_speed)
{
    L1_control((uint16_t)left_speed, 0U);
    R1_control((uint16_t)(right_speed * 0.5f), 0U);
    delay_ms(500U);
    Motor_Stop(1U);
}
