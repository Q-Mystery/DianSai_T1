#include "AllHeader.h"
#include "app_control_config.h"

/* X1 is the logical left-most channel; 1 always means black line. */
uint8_t X1, X2, X3, X4, X5, X6, X7, X8;
int pid_output_IRR;

static int16_t s_last_valid_error;
static int8_t s_previous_error;
static uint8_t s_center_straight;
static uint16_t s_center_stable_cycles;
static uint16_t s_turn_latch_cycles;
static uint16_t s_lost_line_cycles;
static int8_t s_turn_latch_direction;
static int8_t s_last_line_direction;
static uint8_t s_turn_latch_hard;
static uint8_t s_recent_turn_recovery;
static uint8_t s_right_recovery_active;
static uint8_t s_lost_reacquire_cycles;

#define LINE_FAST_STABLE_CYCLES \
    ((uint16_t)((LINE_FAST_STABLE_MS + APP_MAIN_LOOP_DELAY_MS - 1U) / \
                APP_MAIN_LOOP_DELAY_MS))
#define LINE_TURN_LATCH_CYCLES \
    ((uint16_t)((LINE_TURN_LATCH_MS + APP_MAIN_LOOP_DELAY_MS - 1U) / \
                APP_MAIN_LOOP_DELAY_MS))
#define LINE_LOST_RECOVERY_CYCLES \
    ((uint16_t)((LINE_LOST_RECOVERY_MS + APP_MAIN_LOOP_DELAY_MS - 1U) / \
                APP_MAIN_LOOP_DELAY_MS))
#define LINE_LOST_BRAKE_CYCLES \
    ((uint16_t)((LINE_LOST_BRAKE_MS + APP_MAIN_LOOP_DELAY_MS - 1U) / \
                APP_MAIN_LOOP_DELAY_MS))
#define LINE_FAST_AFTER_TURN_STABLE_CYCLES \
    ((uint16_t)((LINE_FAST_AFTER_TURN_STABLE_MS + APP_MAIN_LOOP_DELAY_MS - 1U) / \
                APP_MAIN_LOOP_DELAY_MS))

static int16_t Limit_Wheel_Speed(int16_t speed)
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

static int8_t APP_Line_Sign(int16_t value)
{
    if (value < 0) {
        return -1;
    }
    if (value > 0) {
        return 1;
    }
    return 0;
}

static void APP_Line_Remember_Direction(int8_t direction)
{
    if (direction != 0) {
        s_last_line_direction = direction;
    }
}

static void APP_Line_Arm_Turn_Latch(int8_t direction, uint8_t hard)
{
    if (direction == 0) {
        return;
    }

    s_turn_latch_direction = direction;
    s_turn_latch_cycles = LINE_TURN_LATCH_CYCLES;
    s_turn_latch_hard = hard;
    s_recent_turn_recovery = 1U;
    APP_Line_Remember_Direction(direction);
}

static void APP_Line_Clear_Turn_Latch(void)
{
    s_turn_latch_cycles = 0U;
    s_turn_latch_direction = 0;
    s_turn_latch_hard = 0U;
}

static uint8_t APP_Line_Center_Window_Stable(int8_t error,
                                             uint8_t active_count)
{
    uint8_t center_active = (uint8_t)(X4 || X5);
    uint8_t side_clear = (uint8_t)((X1 == 0U) && (X2 == 0U) &&
                                  (X3 == 0U) && (X6 == 0U) &&
                                  (X7 == 0U) && (X8 == 0U));
    uint8_t centered = (uint8_t)((error >= -LINE_CENTER_DEADBAND) &&
                                (error <= LINE_CENTER_DEADBAND));

    return (uint8_t)(center_active && side_clear && centered &&
                     (active_count <= 2U));
}

static uint8_t APP_Line_Right_Reacquire_Seen(void)
{
    return (uint8_t)(X4 || X5 || X6 || X7 || X8);
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

static void APP_Line_Set_Differential(int8_t direction,
                                      int16_t inner_speed,
                                      int16_t outer_speed,
                                      int16_t trim)
{
    int16_t left_speed;
    int16_t right_speed;

    if (direction < 0) {
        left_speed = (int16_t)(inner_speed - trim);
        right_speed = (int16_t)(outer_speed + trim);
    } else if (direction > 0) {
        left_speed = (int16_t)(outer_speed + trim);
        right_speed = (int16_t)(inner_speed - trim);
    } else {
        left_speed = LINE_BASE_SPEED_MM_S;
        right_speed = LINE_BASE_SPEED_MM_S;
    }

    Motion_Set_Speed(Limit_Wheel_Speed(left_speed),
                     Limit_Wheel_Speed(right_speed));
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
    int16_t base_speed;
    int16_t turn_delta;
    int16_t trim;
    uint16_t fast_stable_target;
    int8_t turn_direction;
    int8_t error;
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
        s_center_stable_cycles = 0U;
        s_center_straight = 0U;
        pid_output_IRR = 0;
        s_right_recovery_active = 1U;
        s_lost_reacquire_cycles = 0U;
        s_turn_latch_direction = 1;
        s_turn_latch_hard = 1U;
        APP_Line_Remember_Direction(1);
        if (s_lost_line_cycles < LINE_LOST_RECOVERY_CYCLES) {
            s_lost_line_cycles++;
            s_recent_turn_recovery = 1U;
            if (s_lost_line_cycles <= LINE_LOST_BRAKE_CYCLES) {
                Motion_Stop(STOP_BRAKE);
            } else {
                APP_Line_Set_Differential(1,
                                          LINE_LOST_RIGHT_SEARCH_INNER_SPEED_MM_S,
                                          LINE_LOST_RIGHT_SEARCH_OUTER_SPEED_MM_S,
                                          0);
            }
        } else {
            Motion_Stop(STOP_BRAKE);
        }
        return;
    }

    if (s_right_recovery_active != 0U) {
        if (APP_Line_Right_Reacquire_Seen() != 0U) {
            if (s_lost_reacquire_cycles < LINE_LOST_REACQUIRE_STABLE_CYCLES) {
                s_lost_reacquire_cycles++;
            }
        } else {
            s_lost_reacquire_cycles = 0U;
        }

        if (s_lost_reacquire_cycles < LINE_LOST_REACQUIRE_STABLE_CYCLES) {
            APP_Line_Set_Differential(1,
                                      LINE_LOST_RIGHT_SEARCH_INNER_SPEED_MM_S,
                                      LINE_LOST_RIGHT_SEARCH_OUTER_SPEED_MM_S,
                                      0);
            return;
        }

        s_right_recovery_active = 0U;
        s_lost_reacquire_cycles = 0U;
    }

    s_lost_line_cycles = 0U;
    pid_output_IRR = (int)APP_HD_PID_Calc(error);
    trim = (int16_t)(myabs(pid_output_IRR) / LINE_PD_TRIM_DIVISOR);

    if ((X1 != 0U) || (X8 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X1, X8, error);
        APP_Line_Arm_Turn_Latch(turn_direction, 1U);
        s_center_straight = 0U;
        s_center_stable_cycles = 0U;
        s_last_valid_error = error;
        APP_Line_Set_Differential(turn_direction,
                                  LINE_HARD_TURN_INNER_SPEED_MM_S,
                                  LINE_HARD_TURN_OUTER_SPEED_MM_S,
                                  trim);
        return;
    }

    if ((X2 != 0U) || (X7 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X2, X7, error);
        APP_Line_Arm_Turn_Latch(turn_direction, 0U);
        s_center_straight = 0U;
        s_center_stable_cycles = 0U;
        s_last_valid_error = error;
        APP_Line_Set_Differential(turn_direction,
                                  LINE_MEDIUM_TURN_INNER_SPEED_MM_S,
                                  LINE_MEDIUM_TURN_OUTER_SPEED_MM_S,
                                  trim);
        return;
    }

    if ((error >= -LINE_CENTER_DEADBAND) &&
        (error <= LINE_CENTER_DEADBAND) &&
        (APP_Line_Center_Window_Stable(error, active_count) != 0U)) {
        APP_Line_Clear_Turn_Latch();
        if (s_center_straight == 0U) {
            /* Remove differential PID history left by the preceding turn. */
            PID_Clear_Motor(MAX_MOTOR);
            s_center_straight = 1U;
        }
        fast_stable_target = (s_recent_turn_recovery != 0U) ?
                                 LINE_FAST_AFTER_TURN_STABLE_CYCLES :
                                 LINE_FAST_STABLE_CYCLES;
        if (s_center_stable_cycles < fast_stable_target) {
            s_center_stable_cycles++;
        }
        s_last_valid_error = error;
        APP_Line_Remember_Direction(APP_Line_Sign(error));
        pid_output_IRR = 0;
        if (s_center_stable_cycles >= fast_stable_target) {
            s_recent_turn_recovery = 0U;
        }
        base_speed = (s_center_stable_cycles >= fast_stable_target) ?
                         LINE_FAST_SPEED_MM_S : LINE_BASE_SPEED_MM_S;
        Motion_Set_Speed(base_speed, base_speed);
        return;
    }

    s_center_straight = 0U;
    s_center_stable_cycles = 0U;
    s_last_valid_error = error;

    if (s_turn_latch_cycles > 0U) {
        s_turn_latch_cycles--;
        turn_direction = s_turn_latch_direction;
        if (turn_direction == 0) {
            turn_direction = APP_Line_Direction_From_Pair(0U, 0U, error);
        }
        if (s_turn_latch_hard != 0U) {
            APP_Line_Set_Differential(turn_direction,
                                      LINE_HARD_TURN_INNER_SPEED_MM_S,
                                      LINE_HARD_TURN_OUTER_SPEED_MM_S,
                                      trim);
        } else {
            APP_Line_Set_Differential(turn_direction,
                                      LINE_MEDIUM_TURN_INNER_SPEED_MM_S,
                                      LINE_MEDIUM_TURN_OUTER_SPEED_MM_S,
                                      trim);
        }
        APP_Line_Remember_Direction(turn_direction);
        return;
    }

    if ((X3 != 0U) || (X6 != 0U)) {
        turn_direction = APP_Line_Direction_From_Pair(X3, X6, error);
        APP_Line_Set_Differential(turn_direction,
                                  LINE_SOFT_TURN_INNER_SPEED_MM_S,
                                  LINE_SOFT_TURN_OUTER_SPEED_MM_S,
                                  trim);
        APP_Line_Remember_Direction(turn_direction);
        return;
    }
    APP_Line_Remember_Direction(APP_Line_Sign(error));

    /*
     * Negative error means the line is left, so the left wheel must slow down.
     * The sign is therefore opposite to the formula used for a right-positive
     * steering command.
     */
    turn_delta = (int16_t)pid_output_IRR;
    left_speed = Limit_Wheel_Speed((int16_t)(LINE_CORNER_SPEED_MM_S + turn_delta));
    right_speed = Limit_Wheel_Speed((int16_t)(LINE_CORNER_SPEED_MM_S - turn_delta));
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
