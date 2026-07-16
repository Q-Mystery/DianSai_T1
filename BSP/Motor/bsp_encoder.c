#include "bsp_encoder.h"
#include "app_control_config.h"

volatile ENCODER_RES motorL_encoder;
volatile ENCODER_RES motorR_encoder;
volatile uint8_t encoder_buf[64];

static bool encoder_window_overspeed(volatile long long count)
{
    if (MOTOR_OVERSPEED_BRAKE_ENABLE == 0U) {
        return false;
    }

    return (count > MOTOR_MAX_PULSES_PER_20MS) ||
           (count < -MOTOR_MAX_PULSES_PER_20MS);
}

void encoder_init(void)
{
    NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
    Timer_20ms_Init();
}

ENCODER_DIR get_encoderL_dir(void)
{
    return motorL_encoder.dir;
}

ENCODER_DIR get_encoderR_dir(void)
{
    return motorR_encoder.dir;
}

void Encoder_Get_ALL(int *encoder_all)
{
    encoder_all[0] = (int)motorL_encoder.ALLcount;
    encoder_all[1] = (int)motorR_encoder.ALLcount;
}

void Encoder_Get_Temp(int *encoder_temp)
{
    encoder_temp[0] = motorL_encoder.count;
    encoder_temp[1] = motorR_encoder.count;
}

void encoder_update(void)
{
    int left_delta = (int)motorL_encoder.temp_count;
    int right_delta = (int)motorR_encoder.temp_count;

    if (ENCODER_E1_REVERSED != 0) {
        left_delta = -left_delta;
    }
    if (ENCODER_E4_REVERSED != 0) {
        right_delta = -right_delta;
    }

    motorL_encoder.count = left_delta;
    motorR_encoder.count = right_delta;
    motorL_encoder.ALLcount += left_delta;
    motorR_encoder.ALLcount += right_delta;
    motorL_encoder.dir = (left_delta >= 0) ? FORWARD : REVERSAL;
    motorR_encoder.dir = (right_delta >= 0) ? FORWARD : REVERSAL;
    motorL_encoder.temp_count = 0;
    motorR_encoder.temp_count = 0;
}

#if MOTOR_ENCODER_IRQ_ENABLE
void GROUP1_IRQHandler(void)
{
    uint32_t pending_source = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

    if ((pending_source & DL_INTERRUPT_GROUP1_IIDX_GPIOB) != 0U) {
        uint32_t gpio_status =
            DL_GPIO_getEnabledInterruptStatus(GPIOB, 0xFFFFFFFFU);

        if ((gpio_status & ENCODER_E1_E1A_PIN) != 0U) {
            if (DL_GPIO_readPins(ENCODER_E1_PORT, ENCODER_E1_E1B_PIN) == 0U) {
                motorL_encoder.temp_count++;
            } else {
                motorL_encoder.temp_count--;
            }
        }

        if ((gpio_status & ENCODER_E1_E1B_PIN) != 0U) {
            if (DL_GPIO_readPins(ENCODER_E1_PORT, ENCODER_E1_E1A_PIN) == 0U) {
                motorL_encoder.temp_count--;
            } else {
                motorL_encoder.temp_count++;
            }
        }

        if ((gpio_status & ENCODER_E4_E4A_PIN) != 0U) {
            if (DL_GPIO_readPins(ENCODER_E4_PORT, ENCODER_E4_E4B_PIN) != 0U) {
                motorR_encoder.temp_count++;
            } else {
                motorR_encoder.temp_count--;
            }
        }

        if ((gpio_status & ENCODER_E4_E4B_PIN) != 0U) {
            if (DL_GPIO_readPins(ENCODER_E4_PORT, ENCODER_E4_E4A_PIN) != 0U) {
                motorR_encoder.temp_count--;
            } else {
                motorR_encoder.temp_count++;
            }
        }

        /* Brake on the first pulse above the configured 20 ms limit. */
        if (encoder_window_overspeed(motorL_encoder.temp_count)) {
            Motor_Brake_Left();
        }
        if (encoder_window_overspeed(motorR_encoder.temp_count)) {
            Motor_Brake_Right();
        }

        DL_GPIO_clearInterruptStatus(
            GPIOB,
            gpio_status & (ENCODER_E1_E1A_PIN |
                           ENCODER_E1_E1B_PIN |
                           ENCODER_E4_E4A_PIN |
                           ENCODER_E4_E4B_PIN));
    }
}
#endif
