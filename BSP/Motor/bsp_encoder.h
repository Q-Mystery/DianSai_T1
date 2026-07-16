#ifndef _BSP_ENCODER_H_
#define _BSP_ENCODER_H_

#include "AllHeader.h"

typedef enum {
    FORWARD,
    REVERSAL
} ENCODER_DIR;

typedef struct {
    volatile long long temp_count;
    volatile int count;
    volatile ENCODER_DIR dir;
    volatile long long ALLcount;
} ENCODER_RES;

void encoder_init(void);
ENCODER_DIR get_encoderL_dir(void);
ENCODER_DIR get_encoderR_dir(void);
void Encoder_Get_ALL(int *encoder_all);
void Encoder_Get_Temp(int *encoder_temp);
void encoder_update(void);

extern volatile uint8_t encoder_buf[64];
extern volatile ENCODER_RES motorL_encoder;
extern volatile ENCODER_RES motorR_encoder;

#endif
