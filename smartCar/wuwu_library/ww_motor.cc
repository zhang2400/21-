#include "ww_motor.h"

Motor::Motor(void)
    : fd(-1)
    , encoder1_counts(0), encoder2_counts(0)
{
    pthread_mutex_init(&data_mutex, NULL);
}

Motor::~Motor(void)
{
    pthread_mutex_destroy(&data_mutex);
    if(fd < 0) return;
    close(fd);
}

/*******************************************************************
 * @brief   电机初始化(调用内核open函数启动pwm信号)
 *          程序调用内核open初始化pwm信号, 程序崩溃自动调用close关闭电机
 * 
 * @return      返回初始化状态
 * @retval      0           初始化成功
 * @retval      -1          初始化失败
 * 
 * @example     //初始化电机 初始化失败退出程序
 *              if(brushless.brushless_init() < 0) {
 *                  return -1;
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
int Motor::motor_init(void)
{
    fd = open(DEVICE_MOTOR_ADDR, O_RDWR);
    if(fd < 0) {
        perror("Error open Motor\r\n");
        return -1;
    } else {
        std::cout << "电机初始化成功" << std::endl;
    }
    return 0;
}

/*******************************************************************
 * @brief   设置电机GPIO电平
 * 
 * @param   gpio_num    电机GPIO仅有2个,设置在设备树(非1及2)
 * @param   value       设置GPIO电平(非1及0)
 * 
 * @example motor.set_gpio_level(1, 0);
 ******************************************************************/
void Motor::set_gpio_level(int gpio_num, int value)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct dir_struct data;
    data.which = gpio_num;
    data.dir = value;
    if(ioctl(fd, MOTOR_SET_DIR, &data)) {
        perror("Failed to set motor gpio level\r\n");
        return;
    }
}

/*******************************************************************
 * @brief   设置电机PWM占空比
 * 
 * @param   pwm_num     电机PWM仅有2个,设置在设备树(非1及2)
 * @param   duty        设置电机PWM控制信号占空比(0~10000)
 * 
 * @example motor.set_pwm_duty(1, 5000);
 ******************************************************************/
void Motor::set_pwm_duty(int pwm_num, int duty)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct pwm_struct data;
    data.which = pwm_num;
    data.duty = duty;
    if(ioctl(fd, MOTOR_SET_PWM, &data)) {
        perror("Failed to set motor pwm duty\r\n");
        return;
    }
}

/*******************************************************************
 * @brief   设置电机1 GPIO信号以及占空比
 *          (直接调用该函数效率高于 set_gpio_level + set_pwm_duty 组合)
 * 
 * @param   level       设置GPIO电平(非1及0)
 * @param   duty        设置电机PWM控制信号占空比(0~10000)
 * 
 * @example motor.set_motor1(0, 5000);
 ******************************************************************/
void Motor::set_motor1(int level, int duty)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }
    
    struct GpioPwm_struct data;
    data.dir = level;
    data.duty = duty;
    if(ioctl(fd, MOTOR_SET_1DIRPWM, &data)) {
        perror("Failed to set motor1\r\n");
        return;
    }
}

/*******************************************************************
 * @brief   设置电机2 GPIO信号以及占空比
 *          (直接调用该函数效率高于 set_gpio_level + set_pwm_duty 组合)
 * 
 * @param   level       设置GPIO电平(非1及0)
 * @param   duty        设置电机PWM控制信号占空比(0~10000)
 * 
 * @example motor.set_motor2(0, 5000);
 ******************************************************************/
void Motor::set_motor2(int level, int duty)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct GpioPwm_struct data;
    data.dir = level;
    data.duty = duty;
    if(ioctl(fd, MOTOR_SET_2DIRPWM, &data)) {
        perror("Failed to set motor2\r\n");
        return;
    }
}

/*******************************************************************
 * @brief   从驱动中读取编码器当前值
 * 
 * @example motor.update_encoders();
 ******************************************************************/
void Motor::update_encoders(void)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct encoder_counts data;
    if(ioctl(fd, ENCODER_GET_COUNT, &data)) {
        perror("Failed to get encoder count\r\n");
        return;
    }
    encoder1_counts = data.encoder1_counts;
    encoder2_counts = data.encoder2_counts;
}

/*******************************************************************
 * @brief       线程同步操作
 * 
 * @example     motor.thread_syn();
 * 
 * @note        在使用syn_xxx数据前, 调用此函数, 用于线程同步, 保证数据安全
 ******************************************************************/
void Motor::thread_syn(void)
{
    pthread_mutex_lock(&data_mutex);
    syn_encoder1_counts = encoder1_counts;
    syn_encoder2_counts = encoder2_counts;
    pthread_mutex_unlock(&data_mutex);
}
