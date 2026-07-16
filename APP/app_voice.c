#include "app_voice.h"

static bool g_voice_ready = false;
static uint8_t g_voice_last_error = 0U;
static uint32_t g_voice_last_alert_ms = 0U;

static void VoiceIIC_Delay(void)
{
    delay_us(VOICE_IIC_DELAY_US);
}

static void VoiceIIC_ReleaseSCL(void)
{
    DL_GPIO_initDigitalInputFeatures(VOICE_IIC_SCL_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

static void VoiceIIC_ReleaseSDA(void)
{
    DL_GPIO_initDigitalInputFeatures(VOICE_IIC_SDA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

static void VoiceIIC_DriveSCLLow(void)
{
    DL_GPIO_initDigitalOutput(VOICE_IIC_SCL_IOMUX);
    DL_GPIO_clearPins(VOICE_IIC_PORT, VOICE_IIC_SCL_PIN);
    DL_GPIO_enableOutput(VOICE_IIC_PORT, VOICE_IIC_SCL_PIN);
}

static void VoiceIIC_DriveSDALow(void)
{
    DL_GPIO_initDigitalOutput(VOICE_IIC_SDA_IOMUX);
    DL_GPIO_clearPins(VOICE_IIC_PORT, VOICE_IIC_SDA_PIN);
    DL_GPIO_enableOutput(VOICE_IIC_PORT, VOICE_IIC_SDA_PIN);
}

static bool VoiceIIC_ReadSDA(void)
{
    return (DL_GPIO_readPins(VOICE_IIC_PORT, VOICE_IIC_SDA_PIN) != 0U);
}

static void VoiceIIC_Start(void)
{
    VoiceIIC_ReleaseSDA();
    VoiceIIC_ReleaseSCL();
    VoiceIIC_Delay();
    VoiceIIC_DriveSDALow();
    VoiceIIC_Delay();
    VoiceIIC_DriveSCLLow();
    VoiceIIC_Delay();
}

static void VoiceIIC_Stop(void)
{
    VoiceIIC_DriveSDALow();
    VoiceIIC_Delay();
    VoiceIIC_ReleaseSCL();
    VoiceIIC_Delay();
    VoiceIIC_ReleaseSDA();
    VoiceIIC_Delay();
}

static bool VoiceIIC_WaitAck(void)
{
    uint8_t wait_count = 0U;

    VoiceIIC_ReleaseSDA();
    VoiceIIC_Delay();
    VoiceIIC_ReleaseSCL();
    VoiceIIC_Delay();

    while (VoiceIIC_ReadSDA()) {
        if (++wait_count >= VOICE_IIC_ACK_WAIT_COUNT) {
            VoiceIIC_DriveSCLLow();
            VoiceIIC_Stop();
            return false;
        }
        VoiceIIC_Delay();
    }

    VoiceIIC_DriveSCLLow();
    VoiceIIC_Delay();
    return true;
}

static bool VoiceIIC_WriteByte(uint8_t data)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        if ((data & 0x80U) != 0U) {
            VoiceIIC_ReleaseSDA();
        } else {
            VoiceIIC_DriveSDALow();
        }
        VoiceIIC_Delay();
        VoiceIIC_ReleaseSCL();
        VoiceIIC_Delay();
        VoiceIIC_DriveSCLLow();
        VoiceIIC_Delay();
        data <<= 1;
    }

    return VoiceIIC_WaitAck();
}

static uint8_t VoiceIIC_WriteRegister(
    uint8_t address, uint8_t reg, uint8_t data)
{
    VoiceIIC_Start();
    if (!VoiceIIC_WriteByte((uint8_t)((address << 1) | 0U))) {
        return 1U;
    }
    if (!VoiceIIC_WriteByte(reg)) {
        return 2U;
    }
    if (!VoiceIIC_WriteByte(data)) {
        return 3U;
    }
    VoiceIIC_Stop();
    return 0U;
}

void AppVoice_Init(void)
{
    g_voice_ready = false;
    g_voice_last_error = 0U;
    g_voice_last_alert_ms = 0U;

    VoiceIIC_ReleaseSCL();
    VoiceIIC_ReleaseSDA();
    delay_ms(VOICE_INIT_DELAY_MS);

    g_voice_ready = AppVoice_SendCommand(VOICE_INIT_COMMAND);
}

bool AppVoice_SendCommand(uint8_t command)
{
    uint8_t retry;

    for (retry = 0U; retry <= VOICE_IIC_RETRY_COUNT; retry++) {
        g_voice_last_error = VoiceIIC_WriteRegister(
            VOICE_IIC_ADDRESS, VOICE_IIC_WRITE_REGISTER, command);
        if (g_voice_last_error == 0U) {
            g_voice_ready = true;
            return true;
        }
        delay_ms(2U);
    }

    g_voice_ready = false;
    return g_voice_ready;
}

bool AppVoice_TriggerObstacle(void)
{
    uint32_t now_ms = Timer_Get_Runtime_Ms();

    if ((g_voice_last_alert_ms != 0U) &&
        ((uint32_t)(now_ms - g_voice_last_alert_ms) <
            VOICE_ALERT_REPEAT_INTERVAL_MS)) {
        return false;
    }

    g_voice_last_alert_ms = now_ms;
    return AppVoice_SendCommand(VOICE_OBSTACLE_COMMAND);
}

bool AppVoice_IsReady(void)
{
    return g_voice_ready;
}

uint8_t AppVoice_GetLastError(void)
{
    return g_voice_last_error;
}
