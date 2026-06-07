#include "motor.h"
#include "circle.h"
#include "turn.h"

motor_param_t motor_l = {0};
motor_param_t motor_r = {0};
pid_param_t motor_pid_l;
pid_param_t motor_pid_r;
int direction_l;
int direction_r;


pid_param_t target_speed_pid = {
    .kp = 5.0f, 
    .ki = 0.0f, 
    .kd = 30.0f,
    .p_max = 5.0f, 
    .i_max = 5.0f,     
    .d_max = 5.0f, 
    .low_pass = 0.6f
};

pid_param_t posloop_pid = {
    .kp = 200.0f, 
    .ki = 0.0f, 
    .kd = 0.0f,
    .p_max = 5000, 
    .i_max = 5000,     
    .d_max = 5000, 
    .low_pass = 0.7f
};

// 舵机PID
// pid_param_t servo_pid;

void motor_PID_init(motor_param_t *motor,
                float target_speed,
                float kp, float ki, float kd,
                float brake_kp, float brake_ki, float brake_kd,
                float low_pass,
                float p_max, float i_max, float d_max)
{
    if (motor == NULL) {
        return;
    }
    
    motor->total_encoder = 0;
    motor->encoder_speed = 0.0f;
    motor->target_speed = target_speed;
    motor->motor_mode = MODE_NORMAL;
    
    pid_init(&motor->pid, kp, ki, kd, low_pass, p_max, i_max, d_max);
    pid_init(&motor->brake_pid, brake_kp, brake_ki, brake_kd, low_pass, p_max, i_max, d_max);
}

// ========== 优化后的速度参数 ==========
float NORMAL_SPEED = 500;      // 基准速度 500 脉冲/秒
float target_speed = 0;

// 速度限制（最终速度范围：200 - 700）
float NORMAL_MAX_SPEED = 200;   // 最大 +200 → 700
float NORMAL_MIN_SPEED = -300;  // 最小 -300 → 200

// 圆环速度
float CIRCLE_MAX_SPEED = 0;
float CIRCLE_MIN_SPEED = -300;   // 圆环时降到 200 脉冲/秒

// 差速系数（调小让转向更平顺）
static const float DIFF_GAIN = 10.0f;  // 原来15.8，改小到10

void speed_control(void) {
    // ========== 基础速度设定 ==========
    float base_speed = 3500.0f;
    
    // ========== 1. 根据角度减速 ==========
    float angle_abs = fabs(angle);
    float target_speed = base_speed - angle_abs * 30.0f;
    
    // ========== 2. 根据路径点数减速 ==========
    if (mid_uinum < 20) {
        target_speed -= 400.0f;
    } else if (mid_uinum < 35) {
        target_speed -= 250.0f;
    } else if (mid_uinum < 50) {
        target_speed -= 100.0f;
    }
    
    // ========== 3. 圆环减速 ==========
    if (circle_type == CIRCLE_LEFT_BEGIN || circle_type == CIRCLE_RIGHT_BEGIN) {
        target_speed -= 600.0f;
    }
    
    // ========== 4. 限幅 ==========
    target_speed = MINMAX(target_speed, 2000.0f, 4000.0f);
    
    // ========== 5. 速度平滑 ==========
    static float last_speed = 3500.0f;
    float max_change = 100.0f;
    float speed_change = target_speed - last_speed;
    speed_change = MINMAX(speed_change, -max_change, max_change);
    target_speed = last_speed + speed_change;
    last_speed = target_speed;
    
    // ========== 6. 取消差速，左右轮速度相同 ==========
    // motor_l.target_speed = target_speed + adc_angle;
    // motor_r.target_speed = target_speed - adc_angle;
    if(adc_angle >0){
        
        motor_l.target_speed = 1000 - adc_angle/2;
        motor_r.target_speed = 1000;
    }
    else{
        motor_l.target_speed = 1000;
        motor_r.target_speed = 1000 + adc_angle/2;
    }   
    
   
}

void motor_control(void) {
    speed_control();

    // 左电机控制
    if (motor_l.motor_mode == MODE_NORMAL) {
        float error = motor_l.target_speed - motor_l.encoder_speed;
        motor_l.duty = pid_solve(&motor_pid_l, error);
        // 修改上下限为 2000-4000
        motor_l.duty = MINMAX(motor_l.duty, -2000, 4000);
    } else if (motor_l.motor_mode == MODE_BANGBANG) {
        motor_pid_l.out_i = 0;
        motor_l.duty += bangbang_pid_solve(&motor_l.brake_pid, motor_l.target_speed - motor_l.encoder_speed);
        // 修改上下限为 2000-4000
        motor_l.duty = MINMAX(motor_l.duty, -2000, 4000);
    } else if (motor_l.motor_mode == MODE_SOFT) {
        motor_pid_l.out_i = 0;
        motor_l.duty += changable_pid_solve(&motor_l.pid, motor_l.target_speed - motor_l.encoder_speed);
        // 修改上下限为 2000-4000
        motor_l.duty = MINMAX(motor_l.duty, -2000, 4000);
    }

    // 右电机控制
    if (motor_r.motor_mode == MODE_NORMAL) {
        float error = motor_r.target_speed - motor_r.encoder_speed;
        motor_r.duty = pid_solve(&motor_pid_r, error);
        // 修改上下限为 2000-4000
        motor_r.duty = MINMAX(motor_r.duty, -2000, 4000);
    } else if (motor_r.motor_mode == MODE_BANGBANG) {
        motor_pid_r.out_i = 0;
        motor_r.duty += bangbang_pid_solve(&motor_r.brake_pid, motor_r.target_speed - motor_r.encoder_speed);
        // 修改上下限为 2000-4000
        motor_r.duty = MINMAX(motor_r.duty, -2000, 4000);
    } else if (motor_r.motor_mode == MODE_SOFT) {
        motor_pid_r.out_i = 0;
        motor_r.duty += changable_pid_solve(&motor_r.pid, motor_r.target_speed - motor_r.encoder_speed);
        // 修改上下限为 2000-4000
        motor_r.duty = MINMAX(motor_r.duty, -2000, 4000);
    }

    // 大角度限幅（保持安全）- 也修改为 2000-4000
    // if (fabs(angle) > 12.0f) {
    //     float limit = 3500;  // 大角度时限制到 3500（在2000-4000范围内）
    //     motor_l.duty = MINMAX(motor_l.duty, 0, limit);
    //     motor_r.duty = MINMAX(motor_r.duty, 0, limit);
    // }

    // PWM输出（现在duty范围是2000-4000，都是正值）
    direction_l = motor_l.duty >= 0 ? 0 : 1;
    direction_r = motor_r.duty >= 0 ? 0 : 1;
    // motor.set_motor1(direction_l, abs(motor_l.duty));
    motor.set_gpio_level(2, direction_l);
    motor.set_pwm_duty(1, motor_l.duty);
    // motor.set_motor2(direction_r, 0);
    motor.set_gpio_level(1, direction_r);
    motor.set_pwm_duty(2, abs(motor_r.duty));
}