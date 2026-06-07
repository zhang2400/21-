#ifndef __WUWU_FANS_H_
#define __WUWU_FANS_H_

#include "headfile.h"

#define FANS_DIR                        "/dev/wuwu_brushless"

class Brushless
{
public:
    Brushless(void);
    ~Brushless(void);

/*******************************************************************
 * @brief       无刷电机初始化
 * 
 * @return      返回初始化状态
 * @retval      0           初始化成功
 * @retval      -1          初始化失败
 * 
 * @example     //初始化电调 初始化失败退出程序
 *              if(brushless.brushless_init() < 0) {
 *                  return -1;
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
    int brushless_init(void);

/*******************************************************************
 * @brief       ZF-STC电调百分比控制
 * 
 * @param       输入范围(0～100) 内核态已对输入做保护
 * 
 * @example     //设置电调占空比50%
 *              brushless.set_duty(50);
 * 
 * @note        无须考虑PWM，内核态已做处理，0%～100%对应PWM高电平1ms ～ 2ms
 *              用户直接调用即可
 ******************************************************************/
    void set_duty(int duty);

/*******************************************************************
 * @brief       获取当前占空比
 * 
 * @return      返回私有数据duty(该数据在set_duty成功是更新)
 * 
 * @example     //打印占空比
 *              std::cout << brushless.get_duty() << std::endl;
 ******************************************************************/
    int get_duty(void);

private:
    // 设备文件描述符
    int fd;
    
    int duty = 0;
};

#endif
