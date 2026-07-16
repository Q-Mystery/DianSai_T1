#include "bsp_PID_motor.h"

PID_t pid_motor[2];

// YAW偏航�?
//YAW yaw angle
PID pid_Yaw = {0, YAW_PID_KP, YAW_PID_KI, YAW_PID_KD, 0, 0, 0};

// 初始化PID参数
//Initialize PID parameters
void PID_Param_Init(void)
{
    /* 速度相关初始化参�?*/
	//Speed dependent initialization parameters
    for (int i = 0; i < MAX_MOTOR; i++)
    {
        pid_motor[i].target_val = 0.0;
        pid_motor[i].pwm_output = 0.0;
        pid_motor[i].err = 0.0;
        pid_motor[i].err_last = 0.0;
        pid_motor[i].err_next = 0.0;
        pid_motor[i].integral = 0.0;

        pid_motor[i].Kp = MOTOR_SPEED_PID_KP;
        pid_motor[i].Ki = MOTOR_SPEED_PID_KI;
        pid_motor[i].Kd = MOTOR_SPEED_PID_KD;
    }
    pid_Yaw.Proportion = YAW_PID_KP;
    pid_Yaw.Integral = YAW_PID_KI;
    pid_Yaw.Derivative = YAW_PID_KD;
}

// Set PID parameters 设置PID参数
void PID_Set_Parm(PID_t *pid, float p, float i, float d)
{
    pid->Kp = p; // Set Scale Factor 设置比例系数 P
    pid->Ki = i; // Set integration coefficient 设置积分系数 I
    pid->Kd = d; // Set differential coefficient 设置微分系数 D
}

// Set the target value of PID 设置PID的目标�?
void PID_Set_Target(PID_t *pid, float temp_val)
{
    pid->target_val = temp_val; // Set the current target value 设置当前的目标�?
}

// Obtain PID target value 获取PID目标�?
float PID_Get_Target(PID_t *pid)
{
    return pid->target_val; // Set the current target value 设置当前的目标�?
}

// Incremental PID calculation formula 增量式PID计算公式
float PID_Incre_Calc(PID_t *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
	//Calculate the error between the target value and the actual value
    pid->err = pid->target_val - actual_val;
    /*PID算法实现*/
    //PID algorithm implementation
    pid->pwm_output += pid->Kp * (pid->err - pid->err_next) + pid->Ki * pid->err + pid->Kd * (pid->err - 2 * pid->err_next + pid->err_last);
    // transmission error
    //transmission error
    pid->err_last = pid->err_next;
    pid->err_next = pid->err;

    // Return PWM output value
    /*Return PWM output value*/

    if (pid->pwm_output > MOTOR_PID_PWM_LIMIT)
        pid->pwm_output = MOTOR_PID_PWM_LIMIT;
    if (pid->pwm_output < (-MOTOR_PID_PWM_LIMIT))
        pid->pwm_output = (-MOTOR_PID_PWM_LIMIT);

    return pid->pwm_output;
}

float PID_Incre_Color_Calc(PID_t *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
	//Calculate the error between the target value and the actual value
    pid->err = actual_val;
    /*PID算法实现*/
    //PID algorithm implementation
    pid->pwm_output += pid->Kp * (pid->err - pid->err_next) + pid->Ki * pid->err + pid->Kd * (pid->err - 2 * pid->err_next + pid->err_last);
    // transmission error
    //transmission error
    pid->err_last = pid->err_next;
    pid->err_next = pid->err;

    // Return PWM output value
    /*Return PWM output value*/

    if (pid->pwm_output > MOTOR_PID_PWM_LIMIT)
        pid->pwm_output = MOTOR_PID_PWM_LIMIT;
    if (pid->pwm_output < (-MOTOR_PID_PWM_LIMIT))
        pid->pwm_output = (-MOTOR_PID_PWM_LIMIT);

    return pid->pwm_output;
}
// 位置式PID计算方式
//Position PID calculation method
float PID_Location_Calc(PID_t *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
	/*Calculate the error between the target value and the actual value*/
    pid->err = pid->target_val - actual_val;

    /* 限定闭环死区 */
    /*Limited closed-loop dead zone*/
    if ((pid->err >= -40) && (pid->err <= 40))
    {
        pid->err = 0;
        pid->integral = 0;
    }

    /* 积分分离，偏差较大时去掉积分作用 */
    /*Integral separation, removing the integral effect when the deviation is large*/
    if (pid->err > -1500 && pid->err < 1500)
    {
        pid->integral += pid->err; // error accumulation 误差累积

        /* Limit the integration range to prevent integration saturation 限定积分范围，防止积分饱�?*/
        if (pid->integral > 4000)
            pid->integral = 4000;
        else if (pid->integral < -4000)
            pid->integral = -4000;
    }

    /*PID算法实现*/ /*PID algorithm implementation*/
    pid->output_val = pid->Kp * pid->err +
                      pid->Ki * pid->integral +
                      pid->Kd * (pid->err - pid->err_last);

    // Error transmission
    pid->err_last = pid->err;

    // Return PID output value
    return pid->output_val;
}

// PID计算输出�?PID calculation output value
void PID_Calc_Motor(motor_data_t *motor)
{
    int i;
    // float pid_out[4] = {0};
    // for (i = 0; i < MAX_MOTOR; i++)
    // {
    //     pid_out[i] = PID_Location_Calc(&pid_motor[i], 0);
    //     PID_Set_Motor_Target(i, pid_out[i]);
    // }

    for (i = 0; i < MAX_MOTOR; i++)
    {
        motor->speed_pwm[i] = PID_Incre_Calc(&pid_motor[i], motor->speed_mm_s[i]);
    }
}

// PID单独计算一条通道 PID calculates one channel separately
float PID_Calc_One_Motor(uint8_t motor_id, float now_speed)
{
    if (motor_id >= MAX_MOTOR)
        return 0;
    return PID_Incre_Calc(&pid_motor[motor_id], now_speed);
}

// 设置PID参数，motor_id=4设置所有，=0123设置对应电机的PID参数�?
//Set PID parameters, motor_ Id=4 Set all,=0123 Set the PID parameters of the corresponding motor.
void PID_Set_Motor_Parm(uint8_t motor_id, float kp, float ki, float kd)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].Kp = kp;
            pid_motor[i].Ki = ki;
            pid_motor[i].Kd = kd;
        }
    }
    else
    {
        pid_motor[motor_id].Kp = kp;
        pid_motor[motor_id].Ki = ki;
        pid_motor[motor_id].Kd = kd;
    }
}

// 清除PID数据
//Clear PID data
void PID_Clear_Motor(uint8_t motor_id)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].pwm_output = 0.0;
            pid_motor[i].err = 0.0;
            pid_motor[i].err_last = 0.0;
            pid_motor[i].err_next = 0.0;
            pid_motor[i].integral = 0.0;
        }
    }
    else
    {
        pid_motor[motor_id].pwm_output = 0.0;
        pid_motor[motor_id].err = 0.0;
        pid_motor[motor_id].err_last = 0.0;
        pid_motor[motor_id].err_next = 0.0;
        pid_motor[motor_id].integral = 0.0;
    }
}

// 设置PID目标速度，单位为：mm/s
//Set PID target speed in mm/s
void PID_Set_Motor_Target(uint8_t motor_id, float target)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].target_val = target;
        }
    }
    else
    {
        pid_motor[motor_id].target_val = target;
    }
}

// 返回PID结构体数�?
//Returns an array of PID structures
PID_t *Pid_Get_Motor(void)
{
    return pid_motor;
}

/*
 ****************结合imu的PID***********
 */
// 重置偏航角的目标�?
//Reset the target value of yaw angle
void PID_Yaw_Reset(float yaw)
{
    pid_Yaw.SetPoint = yaw;
    pid_Yaw.SumError = 0;
    pid_Yaw.LastError = 0;
    pid_Yaw.PrevError = 0;
}

// 计算偏航角的输出�?
//Calculate the output value of yaw angle
float PID_Yaw_Calc(float NextPoint)
{
    float dError, Error;
    Error = pid_Yaw.SetPoint - NextPoint;           // deviation 偏差
    pid_Yaw.SumError += Error;                      // integral 积分
    dError = pid_Yaw.LastError - pid_Yaw.PrevError; // Current differential 当前微分
    pid_Yaw.PrevError = pid_Yaw.LastError;
    pid_Yaw.LastError = Error;

    double omega_rad = pid_Yaw.Proportion * Error            // proportional 比例�?
                       + pid_Yaw.Integral * pid_Yaw.SumError // Integral term 积分�?
                       + pid_Yaw.Derivative * dError;        // differential term 微分�?

    if (omega_rad > PI / 6)
        omega_rad = PI / 6;
    if (omega_rad < -PI / 6)
        omega_rad = -PI / 6;
    return omega_rad;
}

// Set parameters for yaw angle PID 设置偏航角PID的参�?
void PID_Yaw_Set_Parm(float kp, float ki, float kd)
{
    pid_Yaw.Proportion = kp;
    pid_Yaw.Integral = ki;
    pid_Yaw.Derivative = kd;
}
