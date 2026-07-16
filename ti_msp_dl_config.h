/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for motor_PWM */
#define motor_PWM_INST                                                     TIMA0
#define motor_PWM_INST_IRQHandler                               TIMA0_IRQHandler
#define motor_PWM_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define motor_PWM_INST_CLK_FREQ                                         40000000
/* GPIO defines for channel 1 */
#define GPIO_motor_PWM_C1_PORT                                             GPIOB
#define GPIO_motor_PWM_C1_PIN                                     DL_GPIO_PIN_12
#define GPIO_motor_PWM_C1_IOMUX                                  (IOMUX_PINCM29)
#define GPIO_motor_PWM_C1_IOMUX_FUNC                 IOMUX_PINCM29_PF_TIMA0_CCP1
#define GPIO_motor_PWM_C1_IDX                                DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_motor_PWM_C2_PORT                                             GPIOB
#define GPIO_motor_PWM_C2_PIN                                      DL_GPIO_PIN_4
#define GPIO_motor_PWM_C2_IOMUX                                  (IOMUX_PINCM17)
#define GPIO_motor_PWM_C2_IOMUX_FUNC                 IOMUX_PINCM17_PF_TIMA0_CCP2
#define GPIO_motor_PWM_C2_IDX                                DL_TIMER_CC_2_INDEX



/* Defines for TIMER_20ms */
#define TIMER_20ms_INST                                                  (TIMG6)
#define TIMER_20ms_INST_IRQHandler                              TIMG6_IRQHandler
#define TIMER_20ms_INST_INT_IRQN                                (TIMG6_INT_IRQn)
#define TIMER_20ms_INST_LOAD_VALUE                                      (19999U)
/* Defines for TIMER_DISPLAY */
#define TIMER_DISPLAY_INST                                               (TIMA1)
#define TIMER_DISPLAY_INST_IRQHandler                           TIMA1_IRQHandler
#define TIMER_DISPLAY_INST_INT_IRQN                             (TIMA1_INT_IRQn)
#define TIMER_DISPLAY_INST_LOAD_VALUE                                    (3999U)



/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           10000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_10_MHZ_115200_BAUD                                       (5)
#define UART_0_FBRD_10_MHZ_115200_BAUD                                      (27)





/* Port definition for Pin Group OLED */
#define OLED_PORT                                                        (GPIOA)

/* Defines for SCL: GPIOA.8, LaunchPad header pin 4 */
#define OLED_SCL_PIN                                             (DL_GPIO_PIN_8)
#define OLED_SCL_IOMUX                                           (IOMUX_PINCM19)
/* Defines for SDA: GPIOA.22, LaunchPad header pin 24 */
#define OLED_SDA_PIN                                            (DL_GPIO_PIN_22)
#define OLED_SDA_IOMUX                                           (IOMUX_PINCM47)

/* Port definitions for independent Pin Group MPU6050 */
#define MPU6050_I2C_PORT                                                  GPIOB
/* Defines for MPU SCL: GPIOB.2, LaunchPad header pin 9 */
#define MPU6050_I2C_SCL_PIN                                      (DL_GPIO_PIN_2)
#define MPU6050_I2C_SCL_IOMUX                                    (IOMUX_PINCM15)
/* Defines for MPU SDA: GPIOB.3, LaunchPad header pin 10 */
#define MPU6050_I2C_SDA_PIN                                      (DL_GPIO_PIN_3)
#define MPU6050_I2C_SDA_IOMUX                                    (IOMUX_PINCM16)
#define MPU6050_AUX_PORT                                                  GPIOA
/* Defines for MPU XDA: GPIOA.24, LaunchPad header pin 27 */
#define MPU6050_XDA_PIN                                         (DL_GPIO_PIN_24)
#define MPU6050_XDA_IOMUX                                       (IOMUX_PINCM54)
/* Defines for MPU XCL: GPIOA.25, LaunchPad header pin 2 */
#define MPU6050_XCL_PIN                                         (DL_GPIO_PIN_25)
#define MPU6050_XCL_IOMUX                                       (IOMUX_PINCM55)
/* Defines for MPU AD0: GPIOA.26, LaunchPad header pin 5 */
#define MPU6050_AD0_PIN                                         (DL_GPIO_PIN_26)
#define MPU6050_AD0_IOMUX                                       (IOMUX_PINCM59)
/* Defines for MPU INT: GPIOA.27, LaunchPad header pin 8 */
#define MPU6050_INT_PIN                                         (DL_GPIO_PIN_27)
#define MPU6050_INT_IOMUX                                       (IOMUX_PINCM60)
/* Port definition for Pin Group ULTRASONIC */
#define ULTRASONIC_PORT                                                  (GPIOA)
/* Defines for TRIG: GPIOA.15, LaunchPad header pin 30 */
#define ULTRASONIC_TRIG_PIN                                    (DL_GPIO_PIN_15)
#define ULTRASONIC_TRIG_IOMUX                                  (IOMUX_PINCM37)
/* Defines for ECHO: GPIOA.17, LaunchPad header pin 28 */
#define ULTRASONIC_ECHO_PIN                                    (DL_GPIO_PIN_17)
#define ULTRASONIC_ECHO_IOMUX                                  (IOMUX_PINCM39)
/* Port definition for Pin Group VOICE_IIC */
#define VOICE_IIC_PORT                                                   (GPIOA)
/* Defines for SCL: GPIOA.31, LaunchPad header pin 37 */
#define VOICE_IIC_SCL_PIN                                      (DL_GPIO_PIN_31)
#define VOICE_IIC_SCL_IOMUX                                     (IOMUX_PINCM6)
/* Defines for SDA: GPIOA.28, LaunchPad header pin 38 */
#define VOICE_IIC_SDA_PIN                                      (DL_GPIO_PIN_28)
#define VOICE_IIC_SDA_IOMUX                                     (IOMUX_PINCM3)
/* Port definition for Pin Group BCD_DISPLAY */
#define BCD_DISPLAY_PORT                                                (GPIOB)
/* Defines for B0: GPIOB.21 with pinCMx 49 on package pin 20 */
#define BCD_DISPLAY_B0_PIN                                     (DL_GPIO_PIN_21)
#define BCD_DISPLAY_B0_IOMUX                                    (IOMUX_PINCM49)
/* Defines for B1: GPIOB.22 with pinCMx 50 on package pin 21 */
#define BCD_DISPLAY_B1_PIN                                     (DL_GPIO_PIN_22)
#define BCD_DISPLAY_B1_IOMUX                                    (IOMUX_PINCM50)
/* Defines for B2: GPIOB.26 with pinCMx 57 on package pin 28 */
#define BCD_DISPLAY_B2_PIN                                     (DL_GPIO_PIN_26)
#define BCD_DISPLAY_B2_IOMUX                                    (IOMUX_PINCM57)
/* Defines for B3: GPIOB.27 with pinCMx 58 on package pin 29 */
#define BCD_DISPLAY_B3_PIN                                     (DL_GPIO_PIN_27)
#define BCD_DISPLAY_B3_IOMUX                                    (IOMUX_PINCM58)
/* Port definition for Pin Group BCD_DIGIT_SELECT */
#define BCD_DIGIT_SELECT_PORT                                            (GPIOA)
/* Defines for TENS: GPIOA.0 with pinCMx 1 on package pin 33 */
#define BCD_DIGIT_SELECT_TENS_PIN                                (DL_GPIO_PIN_0)
#define BCD_DIGIT_SELECT_TENS_IOMUX                               (IOMUX_PINCM1)
/* Defines for ONES: GPIOA.1 with pinCMx 2 on package pin 34 */
#define BCD_DIGIT_SELECT_ONES_PIN                                (DL_GPIO_PIN_1)
#define BCD_DIGIT_SELECT_ONES_IOMUX                               (IOMUX_PINCM2)
/* Port definition for Pin Group ENCODER_E1 */
#define ENCODER_E1_PORT                                                  (GPIOB)

/* Defines for E1A: GPIOB.8 with pinCMx 25 on package pin 60 */
// groups represented: ["ENCODER_E4","ENCODER_E1"]
// pins affected: ["E4A","E4B","E1A","E1B"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_E1_E1A_IIDX                                  (DL_GPIO_IIDX_DIO8)
#define ENCODER_E1_E1A_PIN                                       (DL_GPIO_PIN_8)
#define ENCODER_E1_E1A_IOMUX                                     (IOMUX_PINCM25)
/* Defines for E1B: GPIOB.7 with pinCMx 24 on package pin 59 */
#define ENCODER_E1_E1B_IIDX                                  (DL_GPIO_IIDX_DIO7)
#define ENCODER_E1_E1B_PIN                                       (DL_GPIO_PIN_7)
#define ENCODER_E1_E1B_IOMUX                                     (IOMUX_PINCM24)
/* Port definition for Pin Group ENCODER_E4 */
#define ENCODER_E4_PORT                                                  (GPIOB)

/* Defines for E4A: GPIOB.20 with pinCMx 48 on package pin 19 */
#define ENCODER_E4_E4A_IIDX                                 (DL_GPIO_IIDX_DIO20)
#define ENCODER_E4_E4A_PIN                                      (DL_GPIO_PIN_20)
#define ENCODER_E4_E4A_IOMUX                                     (IOMUX_PINCM48)
/* Defines for E4B: GPIOB.13 with pinCMx 30 on package pin 1 */
#define ENCODER_E4_E4B_IIDX                                 (DL_GPIO_IIDX_DIO13)
#define ENCODER_E4_E4B_PIN                                      (DL_GPIO_PIN_13)
#define ENCODER_E4_E4B_IOMUX                                     (IOMUX_PINCM30)
/* Port definition for Pin Group Eight_IR */
#define Eight_IR_PORT                                                    (GPIOB)

/* Defines for AD0: GPIOB.16 with pinCMx 33 on package pin 4 */
#define Eight_IR_AD0_PIN                                        (DL_GPIO_PIN_16)
#define Eight_IR_AD0_IOMUX                                       (IOMUX_PINCM33)
/* Defines for AD1: GPIOB.6 with pinCMx 23 on package pin 58 */
#define Eight_IR_AD1_PIN                                         (DL_GPIO_PIN_6)
#define Eight_IR_AD1_IOMUX                                       (IOMUX_PINCM23)
/* Defines for AD2: GPIOB.0 with pinCMx 12 on package pin 47 */
#define Eight_IR_AD2_PIN                                         (DL_GPIO_PIN_0)
#define Eight_IR_AD2_IOMUX                                       (IOMUX_PINCM12)
/* Defines for OUT: GPIOB.1 with pinCMx 13 on package pin 48 */
#define Eight_IR_OUT_PIN                                         (DL_GPIO_PIN_1)
#define Eight_IR_OUT_IOMUX                                       (IOMUX_PINCM13)
/* Defines for AIN1: GPIOB.15 with pinCMx 32 on package pin 3 */
#define MOTOR_DIR_AIN1_PORT                                              (GPIOB)
#define MOTOR_DIR_AIN1_PIN                                      (DL_GPIO_PIN_15)
#define MOTOR_DIR_AIN1_IOMUX                                     (IOMUX_PINCM32)
/* Defines for AIN2: GPIOB.17 with pinCMx 43 on package pin 14 */
#define MOTOR_DIR_AIN2_PORT                                              (GPIOB)
#define MOTOR_DIR_AIN2_PIN                                      (DL_GPIO_PIN_17)
#define MOTOR_DIR_AIN2_IOMUX                                     (IOMUX_PINCM43)
/* Defines for DIN1: GPIOA.12 with pinCMx 34 on package pin 5 */
#define MOTOR_DIR_DIN1_PORT                                              (GPIOA)
#define MOTOR_DIR_DIN1_PIN                                      (DL_GPIO_PIN_12)
#define MOTOR_DIR_DIN1_IOMUX                                     (IOMUX_PINCM34)
/* Defines for DIN2: GPIOA.13 with pinCMx 35 on package pin 6 */
#define MOTOR_DIR_DIN2_PORT                                              (GPIOA)
#define MOTOR_DIR_DIN2_PIN                                      (DL_GPIO_PIN_13)
#define MOTOR_DIR_DIN2_IOMUX                                     (IOMUX_PINCM35)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);
void SYSCFG_DL_motor_PWM_init(void);
void SYSCFG_DL_TIMER_20ms_init(void);
void SYSCFG_DL_TIMER_DISPLAY_init(void);
void SYSCFG_DL_UART_0_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
