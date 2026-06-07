#ifndef _motor_h
#define _motor_h

#include "Img_process.h"
#include "PID.h"
#include "garage.h"
#include "utils.h"
#include "parameter.h"



#define BYTE0(dwTemp) (*(char *)(&dwTemp))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((char *)(&dwTemp) + 3))

// 前轮转角和方向舵机转角的比例关系
#define SMOTOR_RATE     (2.4)
#define MOTOR_PWM_DUTY_MAX    6000
// 枚举定义移到结构体外部
typedef enum {
    MODE_NORMAL,
    MODE_BANGBANG,
    MODE_SOFT,
    MODE_POSLOOP,
} motor_mode_t;

typedef struct motor_param_t {
    float total_encoder;
    float target_encoder;
    float encoder_raw;
    float encoder_speed; //Measured speed
    float target_speed;
    int32_t duty;         //Motor PWM duty
    motor_mode_t motor_mode;  // 使用外部枚举类型
    pid_param_t pid;      //Motor PID param
    pid_param_t brake_pid;      //Motor PID param
} motor_param_t;

// 函数声明：初始化电机参数
void motor_PID_init(motor_param_t *motor,
                float target_speed,
                float kp, float ki, float kd,
                float brake_kp, float brake_ki, float brake_kd,
                float low_pass,
                float p_max, float i_max, float d_max);

extern motor_param_t motor_l, motor_r;
extern pid_param_t motor_pid_l;
extern pid_param_t motor_pid_r;

#define ENCODER_PER_METER   (5800)

void wireless_show(void);

void motor_init(void);

void motor_control(void);


#endif