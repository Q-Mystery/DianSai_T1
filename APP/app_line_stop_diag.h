#ifndef __APP_LINE_STOP_DIAG_H__
#define __APP_LINE_STOP_DIAG_H__

#include <stdint.h>

typedef struct {
    int32_t left_counts;
    int32_t right_counts;
    int32_t error_counts;
    int16_t correction_mm_s;
    int16_t left_target_mm_s;
    int16_t right_target_mm_s;
} LineStop_Diagnostics_t;

void LineStop_GetDiagnostics(LineStop_Diagnostics_t *diagnostics);

#endif
