#ifndef __WUWU_BUZZER_H_
#define __WUWU_BUZZER_H_

#include "headfile.h"

#define DEVICE_BUZZER_ADDR              "/dev/wuwu_buzzer"

class Buzzer
{
public:
    Buzzer(void);
    ~Buzzer(void);

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
    int buzzer_init(void);

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
    void set_duty_freq(int duty_value, int freq_value);

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
    void set_duty(int value);

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
    void set_freq(int value);
private: 
    // 设备文件描述符
    int fd;

    int duty = 0;
    int freq = 0;
};

#endif
