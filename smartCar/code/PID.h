#ifndef _PID_H_
#define _PID_H_


#include "Img_process.h"
#include "garage.h"
#include "utils.h"
#include "cross.h"

#define MINMAX(input, low, upper) MIN(MAX(input, low), upper)

typedef struct {
    float kp;    //P
    float ki;    //I
    float kd;    //D
    float p_max; //integrator_max
    float i_max; //integrator_max    
    float d_max; //integrator_max

    float low_pass;

    float out_p;
    float out_i;
    float out_d;

    float error;
    float pre_error;
    float pre_pre_error;
} pid_param_t;

// 函数声明：初始化PID参数
void pid_init(pid_param_t *pid, 
              float kp, 
              float ki, 
              float kd, 
              float low_pass, 
              float max_p, 
              float max_i, 
              float max_d);


float pid_solve(pid_param_t *pid, float error);

float increment_pid_solve(pid_param_t *pid, float error);

float bangbang_pid_solve(pid_param_t *pid, float error);

float changable_pid_solve(pid_param_t *pid, float error);

extern pid_param_t servo_pid , target_speed_pid, posloop_pid, adc_pid;

#endif /* _PID_H_ */
