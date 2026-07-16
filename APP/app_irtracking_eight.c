#include "AllHeader.h"
#include "app_control_config.h"

/* X1 is the logical left-most channel; 1 always means black line. */
uint8_t X1, X2, X3, X4, X5, X6, X7, X8;
int pid_output_IRR;

static int16_t s_last_valid_error;
static int8_t s_previous_error;
static uint8_t s_center_straight;
static uint16_t s_lost_line_cycles;

static int16_t Limit_Speed(int16_t speed)
{
    if (speed < 0) {
        return 0;
    }
    if (speed > LINE_MAX_WHEEL_SPEED_MM_S) {
        return LINE_MAX_WHEEL_SPEED_MM_S;
    }
    return speed;
}

static int8_t APP_Line_Error_From_Sensors(void)
{
    int8_t left_score;
    int8_t right_score;

    left_score = (int8_t)(X1 * LINE_SCORE_X1 + X2 * LINE_SCORE_X2 +
                          X3 * LINE_SCORE_X3 + X4 * LINE_SCORE_X4);
    right_score = (int8_t)(X8 * LINE_SCORE_X8 + X7 * LINE_SCORE_X7 +
                           X6 * LINE_SCORE_X6 + X5 * LINE_SCORE_X5);

    return (int8_t)(right_score - left_score);
}

static int8_t APP_Line_Direction_From_Pair(uint8_t left_sensor,
                                           uint8_t right_sensor,
                                           int8_t error)
{
    if ((left_sensor != 0U) && (right_sensor == 0U)) {
        return -1;
    }
    if ((right_sensor != 0U) && (left_sensor == 0U)) {
        return 1;
    }
    if (error < 0) {
        return -1;
    }
    if (error > 0) {
        return 1;
    }
    if (s_last_valid_error < 0) {
        return -1;
    }
    if (s_last_valid_error > 0) {
        return 1;
    }
    return 0;
}

float APP_HD_PID_Calc(int8_t actual_value)
{
    float output;

    if ((actual_value >= -LINE_CENTER_DEADBAND) &&
        (actual_value <= LINE_CENTER_DEADBAND)) {
        s_previous_error = 0;
        return 0.0f;
    }

    output = actual_value * LINE_TURN_KP +
             (actual_value - s_previous_error) * LINE_TURN_KD;

    s_previous_error = actual_value;
    if (output > LINE_MAX_TURN_DELTA_MM_S) {
        output = LINE_MAX_TURN_DELTA_MM_S;
    }
    if (output < -LINE_MAX_TURN_DELTA_MM_S) {
        output = -LINE_MAX_TURN_DELTA_MM_S;
    }
    return output;
}

void Copy_HD_Data(void)
{
    X1 = EightIR_IsBlack(0U);
    X2 = EightIR_IsBlack(1U);
    X3 = EightIR_IsBlack(2U);
    X4 = EightIR_IsBlack(3U);
    X5 = EightIR_IsBlack(4U);
    X6 = EightIR_IsBlack(5U);
    X7 = EightIR_IsBlack(6U);
    X8 = EightIR_IsBlack(7U);
}

void deal_IRdata(uint8_t *x1, uint8_t *x2, uint8_t *x3, uint8_t *x4,
                 uint8_t *x5, uint8_t *x6, uint8_t *x7, uint8_t *x8)
{
    Copy_HD_Data();
    if (x1 != NULL) { *x1 = X1; }
    if (x2 != NULL) { *x2 = X2; }
    if (x3 != NULL) { *x3 = X3; }
    if (x4 != NULL) { *x4 = X4; }
    if (x5 != NULL) { *x5 = X5; }
    if (x6 != NULL) { *x6 = X6; }
    if (x7 != NULL) { *x7 = X7; }
    if (x8 != NULL) { *x8 = X8; }
}

int LineCheck(void)
{
    return (X1 || X2 || X3 || X4 || X5 || X6 || X7 || X8) ? BLACK : WHITE;
}

void LineWalking(void)
{
    uint8_t sensors[8];
    uint8_t active_count = 0U;
    int16_t left_speed;
    int16_t right_speed;
    int8_t error;
    int8_t turn_direction;
    uint8_t i;

    ReadEightIR(IR_Data_number);
    Copy_HD_Data();

    sensors[0] = X1;
    sensors[1] = X2;
    sensors[2] = X3;
    sensors[3] = X4;
    sensors[4] = X5;
    sensors[5] = X6;
    sensors[6] = X7;
    sensors[7] = X8;

    for (i = 0U; i < 8U; i++) {
        if (sensors[i] != 0U) {
            active_count++;
        }
    }

    error = APP_Line_Error_From_Sensors();

    if (active_count == 0U) {
        s_previous_error = 0;
        pid_output_IRR = 0;
        if (s_lost_line_cycles < LINE_LOST_FORWARD_CYCLES) {
            s_lost_line_cycles++;
            if (s_last_valid_error < 0) {
                Motion_Set_Speed(LINE_TURN_INNER_SPEED_MM_S,
                                 LINE_SEARCH_SPEED_MM_S);
            } else if (s_last_valid_error > 0) {
                Motion_Set_Speed(LINE_SEARCH_SPEED_MM_S,
                                 LINE_TURN_INNER_SPEED_MM_S);
            } else {
                Motion_Set_Speed(LINE_SEARCH_SPEED_MM_S,
                                 LINE_SEARCH_SPEED_MM_S);
            }
        } else {
            Motion_Set_Speed(0, 0);
        }
        return;
    }

    s_lost_line_cycles = 0U;

    if (((X4 != 0U) || (X5 != 0U)) &&
        ((X1 == 0U) && (X2 == 0U) && (X7 == 0U) && (X8 == 0U))) {
        if (s_center_straight == 0U) {
            PID_Clear_Motor(MAX_MOTOR);
            s_center_straight = 1U;
        }
        s_last_valid_error = error;
        s_previous_error = 0;
        pid_output_IRR = 0;
        Motion_Set_Speed(LINE_BASE_SPEED_MM_S, LINE_BASE_SPEED_MM_S);
        return;
    }

    if ((error >= -LINE_CENTER_DEADBAND) &&
        (error <= LINE_CENTER_DEADBAND)) {
        if (s_center_straight == 0U) {
            /* Remove differential PID history left by the preceding turn. */
            PID_Clear_Motor(MAX_MOTOR);
            s_center_straight = 1U;
        }
        s_last_valid_error = error;
        s_previous_error = 0;
        pid_output_IRR = 0;
        Motion_Set_Speed(LINE_BASE_SPEED_MM_S, LINE_BASE_SPEED_MM_S);
        return;
    }

    s_center_straight = 0U;
    s_last_valid_error = error;
    pid_output_IRR = (int)APP_HD_PID_Calc(error);

    if ((X1 != 0U) || (X8 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X1, X8, error);
        if (turn_direction < 0) {
            left_speed = LINE_HARD_TURN_INNER_SPEED_MM_S;
            right_speed = LINE_HARD_TURN_OUTER_SPEED_MM_S;
        } else if (turn_direction > 0) {
            left_speed = LINE_HARD_TURN_OUTER_SPEED_MM_S;
            right_speed = LINE_HARD_TURN_INNER_SPEED_MM_S;
        } else {
            left_speed = LINE_BASE_SPEED_MM_S;
            right_speed = LINE_BASE_SPEED_MM_S;
        }
    } else if ((X2 != 0U) || (X7 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X2, X7, error);
        if (turn_direction < 0) {
            left_speed = LINE_ARC_TURN_INNER_SPEED_MM_S;
            right_speed = LINE_TURN_OUTER_SPEED_MM_S;
        } else if (turn_direction > 0) {
            left_speed = LINE_TURN_OUTER_SPEED_MM_S;
            right_speed = LINE_ARC_TURN_INNER_SPEED_MM_S;
        } else {
            left_speed = LINE_BASE_SPEED_MM_S;
            right_speed = LINE_BASE_SPEED_MM_S;
        }
    } else if ((X3 != 0U) || (X6 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X3, X6, error);
        if (turn_direction < 0) {
            left_speed = LINE_SOFT_TURN_INNER_SPEED_MM_S;
            right_speed = LINE_SOFT_TURN_OUTER_SPEED_MM_S;
        } else if (turn_direction > 0) {
            left_speed = LINE_SOFT_TURN_OUTER_SPEED_MM_S;
            right_speed = LINE_SOFT_TURN_INNER_SPEED_MM_S;
        } else {
            left_speed = LINE_BASE_SPEED_MM_S;
            right_speed = LINE_BASE_SPEED_MM_S;
        }
    } else if (error < 0) {
        left_speed = LINE_TURN_INNER_SPEED_MM_S;
        right_speed = (myabs(error) >= 5) ?
                          LINE_HARD_TURN_OUTER_SPEED_MM_S :
                          LINE_TURN_OUTER_SPEED_MM_S;
    } else {
        left_speed = (myabs(error) >= 5) ?
                         LINE_HARD_TURN_OUTER_SPEED_MM_S :
                         LINE_TURN_OUTER_SPEED_MM_S;
        right_speed = LINE_TURN_INNER_SPEED_MM_S;
    }

    left_speed = Limit_Speed(left_speed);
    right_speed = Limit_Speed(right_speed);
    Motion_Set_Speed(left_speed, right_speed);
}

void LineWalking_PWM(void)
{
    LineWalking();
}

void Line_Tracke(void)
{
    LineWalking();
}
