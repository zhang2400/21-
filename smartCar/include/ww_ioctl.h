#ifndef __WUWU_IOCRL_H_
#define __WUWU_IOCRL_H_

#include "headfile.h"

/* 本文件均为内部调用，用于用户态和内核态通信，修改将导致无法与内核态通信导致程序异常(请勿修改！) */
/* 本文件均为内部调用，用于用户态和内核态通信，修改将导致无法与内核态通信导致程序异常(请勿修改！) */
/* 本文件均为内部调用，用于用户态和内核态通信，修改将导致无法与内核态通信导致程序异常(请勿修改！) */

/***********************************************************************************
 * @brief           电机IOCTL
 * 
 * @param           MOTOR_IOCTL_MAGIC       IOCTL魔数(不可重复)
 * @param           MOTOR_SET_DIR           电机dir方向修改CMD
 * @param           MOTOR_SET_PWM           电机pwm占空比修改CMD
 * @param           MOTOR_SET_1DIRPWM       电机1 dir-pwm 修改CMD
 * @param           MOTOR_SET_2DIRPWM       电机2 dir-pwm 修改CMD
 * @param           ENCODER_GET_COUNT       编码器获取CMD
 * 
 * @param           dir_struct              控制电机dir信号的传递结构体
 * @param           pwm_struct              控制电机pwm信号的传递结构体
 * @param           GpioPwm_struct          控制电机dir和pwm信号的传递结构体
 * @param           encoder_counts          获取编码器的传递结构体
 * 
 * @note            以下内容均为内部调用，无须修改，修改以下任何参数，修改将导致电机无法正常使用
 *                           
 */
#define MOTOR_IOCTL_MAGIC               'O'
#define MOTOR_SET_DIR                   _IOW(MOTOR_IOCTL_MAGIC, 1, struct dir_struct)
#define MOTOR_SET_PWM                   _IOW(MOTOR_IOCTL_MAGIC, 2, struct pwm_struct)
#define MOTOR_SET_1DIRPWM               _IOW(MOTOR_IOCTL_MAGIC, 3, struct GpioPwm_struct)
#define MOTOR_SET_2DIRPWM               _IOW(MOTOR_IOCTL_MAGIC, 4, struct GpioPwm_struct)
#define ENCODER_GET_COUNT               _IOR(MOTOR_IOCTL_MAGIC, 5, struct encoder_counts)

struct dir_struct {
    int which;
    int dir;
};
struct pwm_struct {
    int which;
    int duty;
};
struct GpioPwm_struct {
    int dir;
    int duty;
};
struct encoder_counts {
    int encoder1_counts;
    int encoder2_counts;
};

/***********************************************************************************
 * @brief           ICM42688IOCTL
 * 
 * @param           ICM42688_IOCTL_MAGIC        IOCTL魔数(不可重复)
 * @param           ICM42688_GET_ACCEL          ICM42688获取加速度计数据CMD
 * @param           ICM42688_GET_GYRO           ICM42688获取陀螺仪数据CMD
 * 
 * @param           icm42688_accel_data         获取ICM42688加速度计数据
 * @param           icm42688_gyro_data          获取ICM42688陀螺仪数据
 * 
 * @note            以下内容均为内部调用，无须修改，修改以下任何参数，修改将导致ICM42688无法正常使用
 */
#define ICM42688_IOCTL_MAGIC            'I'
#define ICM42688_GET_ACCEL              _IOR(ICM42688_IOCTL_MAGIC, 1, struct icm42688_accel_data)
#define ICM42688_GET_GYRO               _IOR(ICM42688_IOCTL_MAGIC, 2, struct icm42688_gyro_data)

struct icm42688_accel_data {
    short x, y, z;
};
struct icm42688_gyro_data {
    short x, y, z;
};

/***********************************************************************************
 * @brief           无缘蜂鸣器IOCTL
 * 
 * @param           BUZEER_IOCTL_MAGIC          IOCTL魔数(不可重复)
 * @param           BUZEER_SET_DUTYHZ           蜂鸣器占空比-频率设置
 * 
 * @param           Buzzer_pwm                  无缘蜂鸣器频率-占空比传递结构体
 * 
 * @note            以下内容均为内部调用，无须修改，修改以下任何参数，修改将导致蜂鸣器无法正常使用
 */
#define BUZEER_IOCTL_MAGIC              'b'
#define BUZEER_SET_DUTYHZ               _IOW(BUZEER_IOCTL_MAGIC, 1, struct Buzzer_pwm)

struct Buzzer_pwm {
    int hz;
    int duty;
};

/***********************************************************************************
 * @brief           ZF无刷电调PWM控制IOCTL
 * 
 * @param           BRUSHLESS_IOCTL_MAGIC       IOCTL魔数(不可重复)
 * @param           BRUSHLESS_SET_DUTY          无刷电调控制
 * 
 * @param           Brushless_duty              无刷电调百分比控制
 * 
 * @note            以下内容均为内部调用，无须修改，修改以下任何参数，修改将导致电调无法正常使用
 */
#define BRUSHLESS_IOCTL_MAGIC           'B'
#define BRUSHLESS_SET_DUTY              _IOW(BRUSHLESS_IOCTL_MAGIC, 1, struct Brushless_duty)

struct Brushless_duty {
    int duty;
};

/***********************************************************************************
 * @brief           TOF读取IOCTL
 * 
 * @param           VL53L0X_IOCTL_MAGIC         IOCTL魔数(不可重复)
 * @param           VL53L0X_GET_DATA            TOF数据获取
 * 
 * @note            以下内容均为内部调用，无须修改，修改以下任何参数，修改将导致TOF无法正常使用
 */
#define VL53L0X_IOCTL_MAGIC             't'
#define VL53L0X_GET_DATA                _IOR(VL53L0X_IOCTL_MAGIC, 1, uint16_t)


#endif
