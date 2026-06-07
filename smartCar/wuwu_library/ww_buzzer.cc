#include "ww_buzzer.h"

/**
 * 
 */
Buzzer::Buzzer(void)
    : fd(-1)
{

}

Buzzer::~Buzzer(void)
{
    if(fd < 0) return;
    close(fd);
}

/*******************************************************************
 * @brief       蜂鸣器初始化
 * 
 * @return      返回初始化状态
 * @retval      0               初始化成功
 * @retval      -1              初始化失败
 * 
 * @example     //蜂鸣器初始化
 *              if(buzzer.buzzer_init() < 0) {
 *                  return -1;             
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
int Buzzer::buzzer_init(void)
{
    fd = open(DEVICE_BUZZER_ADDR, O_WRONLY);
    if(fd < 0) {
        perror("Error open Buzzer\r\n");
        return -1;
    } else {
        std::cout << "蜂鸣器已打开" << std::endl;
    }
    return 0;
}

/*******************************************************************
 * @brief       设置蜂鸣器占空比-频率
 * 
 * @param       duty_value      设置占空比(0~100%)
 * @param       freq_value      设置蜂鸣器频率
 * 
 * @example     //设置频率500HZ 50%占空比
 *              buzzer.set_duty_freq(50, 500);
 * 
 * @note        请设置人耳可以听到的频率，否则听不到声音
 ******************************************************************/
void Buzzer::set_duty(int value)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct Buzzer_pwm buzzer_data;
    buzzer_data.duty = value;
    buzzer_data.hz = freq;
    if(ioctl(fd, BUZEER_SET_DUTYHZ, &buzzer_data)) {
        perror("Failed to set buzzer duty ioctl\r\n");
        return;
    }

    duty = value;
}

/*******************************************************************
 * @brief       设置蜂鸣器占空比
 * 
 * @param       value           设置占空比(0~100%)
 * 
 * @example     //设置占空比
 *              buzzer.set_duty(50);
 * 
 * @note        无缘蜂鸣器只考虑输入频率, 与占空比无关 占空比0和100都无法响
 ******************************************************************/
void Buzzer::set_freq(int value)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct Buzzer_pwm buzzer_data;
    buzzer_data.duty = duty;
    buzzer_data.hz = value;
    if(ioctl(fd, BUZEER_SET_DUTYHZ, &buzzer_data)) {
        perror("Failed to set buzzer freq ioctl\r\n");
        return;
    }

    freq = value;
}

/*******************************************************************
 * @brief       设置蜂鸣器频率
 * 
 * @param       value           设置蜂鸣器频率
 * 
 * @example     //设置频率500HZ
 *              buzzer.set_freq(500);
 * 
 * @note        请设置人耳可以听到的频率，否则听不到声音
 ******************************************************************/
void Buzzer::set_duty_freq(int duty_value, int freq_value)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct Buzzer_pwm buzzer_data;
    buzzer_data.duty = duty_value;
    buzzer_data.hz = freq_value;
    if(ioctl(fd, BUZEER_SET_DUTYHZ, &buzzer_data)) {
        perror("Failed to set buzzer freq duty ioctl\r\n");
        return;
    }

    duty = duty_value;
    freq = freq_value;
}

