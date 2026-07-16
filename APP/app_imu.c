#include "app_imu.h"
#include "AllHeader.h"

#define MPU6050_REG_SMPLRT_DIV     (0x19U)
#define MPU6050_REG_CONFIG         (0x1AU)
#define MPU6050_REG_GYRO_CONFIG    (0x1BU)
#define MPU6050_REG_ACCEL_CONFIG   (0x1CU)
#define MPU6050_REG_FIFO_EN        (0x23U)
#define MPU6050_REG_INT_ENABLE     (0x38U)
#define MPU6050_REG_GYRO_XOUT_H    (0x43U)
#define MPU6050_REG_USER_CTRL      (0x6AU)
#define MPU6050_REG_PWR_MGMT_1     (0x6BU)
#define MPU6050_REG_PWR_MGMT_2     (0x6CU)
#define MPU6050_REG_WHO_AM_I       (0x75U)

#define MPU6050_WRITE_ADDRESS      ((uint8_t)(IMU_I2C_ADDRESS << 1U))
#define MPU6050_READ_ADDRESS       ((uint8_t)((IMU_I2C_ADDRESS << 1U) | 1U))

static AppIMU_Status_t g_imu_status;

static void IMU_I2C_Delay(void)
{
    delay_us(3);
}

static void IMU_SCL_Low(void)
{
    DL_GPIO_clearPins(MPU6050_I2C_PORT, MPU6050_I2C_SCL_PIN);
    DL_GPIO_enableOutput(MPU6050_I2C_PORT, MPU6050_I2C_SCL_PIN);
}

static void IMU_SCL_Release(void)
{
    DL_GPIO_disableOutput(MPU6050_I2C_PORT, MPU6050_I2C_SCL_PIN);
}

static void IMU_SDA_Low(void)
{
    DL_GPIO_clearPins(MPU6050_I2C_PORT, MPU6050_I2C_SDA_PIN);
    DL_GPIO_enableOutput(MPU6050_I2C_PORT, MPU6050_I2C_SDA_PIN);
}

static void IMU_SDA_Release(void)
{
    DL_GPIO_disableOutput(MPU6050_I2C_PORT, MPU6050_I2C_SDA_PIN);
}

static bool IMU_SDA_Read(void)
{
    return (DL_GPIO_readPins(MPU6050_I2C_PORT, MPU6050_I2C_SDA_PIN) != 0U);
}

static void IMU_I2C_InitPins(void)
{
    DL_GPIO_initDigitalInputFeatures(MPU6050_I2C_SCL_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(MPU6050_I2C_SDA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(MPU6050_XDA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(MPU6050_XCL_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalOutput(MPU6050_AD0_IOMUX);
    DL_GPIO_clearPins(MPU6050_AUX_PORT, MPU6050_AD0_PIN);
    DL_GPIO_enableOutput(MPU6050_AUX_PORT, MPU6050_AD0_PIN);
    DL_GPIO_initDigitalInputFeatures(MPU6050_INT_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    IMU_SCL_Release();
    IMU_SDA_Release();
}

static void IMU_I2C_Start(void)
{
    IMU_SDA_Release();
    IMU_SCL_Release();
    IMU_I2C_Delay();
    IMU_SDA_Low();
    IMU_I2C_Delay();
    IMU_SCL_Low();
    IMU_I2C_Delay();
}

static void IMU_I2C_Stop(void)
{
    IMU_SDA_Low();
    IMU_I2C_Delay();
    IMU_SCL_Release();
    IMU_I2C_Delay();
    IMU_SDA_Release();
    IMU_I2C_Delay();
}

static bool IMU_I2C_WaitAck(void)
{
    uint16_t timeout = 80U;

    IMU_SDA_Release();
    IMU_I2C_Delay();
    IMU_SCL_Release();
    IMU_I2C_Delay();
    while (IMU_SDA_Read()) {
        if (timeout == 0U) {
            IMU_SCL_Low();
            return false;
        }
        timeout--;
        delay_us(1);
    }
    IMU_SCL_Low();
    IMU_I2C_Delay();
    return true;
}

static void IMU_I2C_SendAck(bool ack)
{
    if (ack) {
        IMU_SDA_Low();
    } else {
        IMU_SDA_Release();
    }
    IMU_I2C_Delay();
    IMU_SCL_Release();
    IMU_I2C_Delay();
    IMU_SCL_Low();
    IMU_SDA_Release();
}

static void IMU_I2C_WriteByte(uint8_t data)
{
    uint8_t bit;

    for (bit = 0U; bit < 8U; bit++) {
        if ((data & 0x80U) != 0U) {
            IMU_SDA_Release();
        } else {
            IMU_SDA_Low();
        }
        IMU_I2C_Delay();
        IMU_SCL_Release();
        IMU_I2C_Delay();
        IMU_SCL_Low();
        data <<= 1U;
    }
    IMU_SDA_Release();
}

static uint8_t IMU_I2C_ReadByte(bool ack)
{
    uint8_t bit;
    uint8_t data = 0U;

    IMU_SDA_Release();
    for (bit = 0U; bit < 8U; bit++) {
        data <<= 1U;
        IMU_SCL_Release();
        IMU_I2C_Delay();
        if (IMU_SDA_Read()) {
            data |= 1U;
        }
        IMU_SCL_Low();
        IMU_I2C_Delay();
    }
    IMU_I2C_SendAck(ack);
    return data;
}

static bool IMU_WriteReg(uint8_t reg, uint8_t value)
{
    bool ok = true;

    IMU_I2C_Start();
    IMU_I2C_WriteByte(MPU6050_WRITE_ADDRESS);
    ok &= IMU_I2C_WaitAck();
    IMU_I2C_WriteByte(reg);
    ok &= IMU_I2C_WaitAck();
    IMU_I2C_WriteByte(value);
    ok &= IMU_I2C_WaitAck();
    IMU_I2C_Stop();
    return ok;
}

static bool IMU_ReadBytes(uint8_t reg, uint8_t *data, uint8_t length)
{
    uint8_t i;
    bool ok = true;

    IMU_I2C_Start();
    IMU_I2C_WriteByte(MPU6050_WRITE_ADDRESS);
    ok &= IMU_I2C_WaitAck();
    IMU_I2C_WriteByte(reg);
    ok &= IMU_I2C_WaitAck();

    IMU_I2C_Start();
    IMU_I2C_WriteByte(MPU6050_READ_ADDRESS);
    ok &= IMU_I2C_WaitAck();
    if (!ok) {
        IMU_I2C_Stop();
        return false;
    }

    for (i = 0U; i < length; i++) {
        data[i] = IMU_I2C_ReadByte(i < (uint8_t)(length - 1U));
    }
    IMU_I2C_Stop();
    return true;
}

static bool IMU_ReadReg(uint8_t reg, uint8_t *value)
{
    return IMU_ReadBytes(reg, value, 1U);
}

static bool IMU_ReadGyroZRaw(int16_t *gyro_z)
{
    uint8_t data[6];

    if (!IMU_ReadBytes(MPU6050_REG_GYRO_XOUT_H, data, sizeof(data))) {
        return false;
    }
    *gyro_z = (int16_t)(((uint16_t)data[4] << 8U) | data[5]);
    return true;
}

static bool IMU_Configure(void)
{
    bool ok = true;

    ok &= IMU_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x80U);
    delay_ms(100);
    ok &= IMU_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01U);
    ok &= IMU_WriteReg(MPU6050_REG_PWR_MGMT_2, 0x00U);
    ok &= IMU_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x18U);
    ok &= IMU_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x00U);
    ok &= IMU_WriteReg(MPU6050_REG_CONFIG, 0x03U);
    ok &= IMU_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x04U);
    ok &= IMU_WriteReg(MPU6050_REG_FIFO_EN, 0x00U);
    ok &= IMU_WriteReg(MPU6050_REG_INT_ENABLE, 0x00U);
    ok &= IMU_WriteReg(MPU6050_REG_USER_CTRL, 0x00U);
    delay_ms(20);
    return ok;
}

static void IMU_CalibrateGyroZ(void)
{
    uint8_t i;
    uint8_t valid = 0U;
    int32_t sum = 0;
    int16_t gyro_z;

    for (i = 0U; i < IMU_CALIBRATION_SAMPLES; i++) {
        if (IMU_ReadGyroZRaw(&gyro_z)) {
            sum += gyro_z;
            valid++;
        }
        delay_ms(2);
    }

    if (valid > 0U) {
        g_imu_status.gyro_z_offset = (int16_t)(sum / valid);
    } else {
        g_imu_status.available = false;
    }
}

bool AppIMU_Init(void)
{
    uint8_t retry;
    uint8_t who = 0U;

    memset(&g_imu_status, 0, sizeof(g_imu_status));
    IMU_I2C_InitPins();

    for (retry = 0U; retry < IMU_INIT_RETRY_COUNT; retry++) {
        if (IMU_Configure() &&
            IMU_ReadReg(MPU6050_REG_WHO_AM_I, &who) &&
            ((who == 0x68U) || (who == 0x70U))) {
            g_imu_status.available = true;
            g_imu_status.who_am_i = who;
            IMU_CalibrateGyroZ();
            AppIMU_Update();
            return g_imu_status.available;
        }
        delay_ms(20);
    }

    g_imu_status.available = false;
    return false;
}

void AppIMU_Update(void)
{
    uint32_t now_ms;
    int32_t corrected;
    int16_t gyro_z;

    if (!g_imu_status.available) {
        return;
    }

    now_ms = Timer_Get_Runtime_Ms();
    if ((g_imu_status.last_update_ms != 0U) &&
        ((uint32_t)(now_ms - g_imu_status.last_update_ms) <
            IMU_UPDATE_MIN_INTERVAL_MS)) {
        return;
    }

    if (!IMU_ReadGyroZRaw(&gyro_z)) {
        g_imu_status.available = false;
        return;
    }

    corrected = (int32_t)gyro_z - (int32_t)g_imu_status.gyro_z_offset;
    g_imu_status.gyro_z_raw = gyro_z;
    g_imu_status.yaw_rate_dps_x10 =
        (int16_t)((corrected * 10) / IMU_GYRO_Z_DPS_X10_DIVISOR);
    g_imu_status.is_curve =
        (corrected > IMU_GYRO_Z_CURVE_THRESHOLD_RAW) ||
        (corrected < -IMU_GYRO_Z_CURVE_THRESHOLD_RAW);
    g_imu_status.last_update_ms = now_ms;
}

const AppIMU_Status_t *AppIMU_GetStatus(void)
{
    return &g_imu_status;
}
