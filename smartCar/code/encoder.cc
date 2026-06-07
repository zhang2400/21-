#include "encoder.h"
#include "motor.h"

extern motor_param_t motor_l, motor_r;
extern Motor motor;

float low_pass = 0.5;

void encoder_get(void) {
    // 读取编码器硬件数据
    motor.update_encoders();
    motor.thread_syn();
    
    // 获取编码器增量值
    int32_t delta_l = motor.syn_encoder1_counts;
    int32_t delta_r = motor.syn_encoder2_counts;
    
    // 假设每10ms调用一次（由线程周期保证）
    float dt = 0.02;  // 10ms = 0.01秒
    
    // 计算速度（脉冲/秒）
    float speed_l_raw = delta_l / dt;
    float speed_r_raw = delta_r / dt;
    
    // 低通滤波
    motor_l.encoder_speed = motor_l.encoder_speed * low_pass + speed_l_raw * (1 - low_pass);
    motor_r.encoder_speed = motor_r.encoder_speed * low_pass + speed_r_raw * (1 - low_pass);
    
    // 累计总脉冲（用于位置环）
    motor_l.total_encoder += delta_l;
    motor_r.total_encoder += delta_r;
}