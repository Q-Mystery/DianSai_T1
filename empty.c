#include "ti_msp_dl_config.h"

#include "app_irtracking.h"
#include "app_motor.h"
#include "app_mpu6050.h"
#include "bsp_motor.h"
#include "bsp_mpu6050.h"
#include "buzzer.h"
#include "delay.h"
#include "inv_mpu.h"
#include "key.h"
#include "led.h"
#include "questions.h"
#include "task.h"
#include "task_profile.h"
#include "usart.h"

int g_LinePortal_flag = 0;
char buffer[80];

int main(void)
{
    USART_Init();
    Motor_Init();

    MPU6050_Init();
    while (mpu_dmp_init()) {
        printf("dmp error\r\n");
        delay_ms(200);
    }

    PWM_Buzzer_Init();
    State_Machine_SelectTask(ACTIVE_QUESTION_ID);
    /* Standalone firmware: start the assigned task without a switch. */
    g_LinePortal_flag = 1;

    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_0_INST);

    while (1) {
        Scheduler_Run();

        if (g_LinePortal_flag) {
            if (State_Machine.Main_State == ACTIVE_QUESTION_ID) {
                ACTIVE_QUESTION_FUNCTION();
            } else {
                Motor_Stop();
            }
        }
    }
}
