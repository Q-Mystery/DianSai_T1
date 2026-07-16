#ifndef __APP_VOICE_H__
#define __APP_VOICE_H__

#include "AllHeader.h"

void AppVoice_Init(void);
bool AppVoice_SendCommand(uint8_t command);
bool AppVoice_TriggerObstacle(void);
bool AppVoice_IsReady(void);
uint8_t AppVoice_GetLastError(void);

#endif
