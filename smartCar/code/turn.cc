#include "turn.h"
#include "parameter.h"
#include "PID.h"
#include "motor.h"

// 坡道状态
float adc_angle = 0;

pid_param_t adc_pid = {
    .kp = 100.0f, 
    .ki = 0.2f, 
    .kd = 30.f,
    .p_max = 50.0f, 
    .i_max = 30.0f,      
    .d_max = 40.0f, 
    .low_pass = 0.8f
};

void elec_calculate(void) {
    adc_angle = pid_solve(&adc_pid, angle);
    adc_angle = MINMAX(adc_angle, -2000, 2000);
    
    // // 左转（angle为正）：左轮减速，右轮加速
    // motor.set_motor1(0, MINMAX(motor_l.duty - adc_angle, 2000, 4000));
    // motor.set_motor2(0, MINMAX(motor_r.duty + adc_angle, 2000, 4000));
}