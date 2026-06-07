#include "ww_brushless.h"

Brushless::Brushless(void)
    : fd(-1)
{

}

Brushless::~Brushless(void)
{
    if(fd < 0) return;
    close(fd);
}

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
int Brushless::brushless_init(void)
{
    fd = open(FANS_DIR, O_WRONLY);
    if(fd < 0) {
        perror("Error open brushless\r\n");
        return -1;
    } else {
        std::cout << "无刷电机初始化成功" << std::endl;
    }
    return 0;
}

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
void Brushless::set_duty(int value)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct Brushless_duty brushless_duty;
    value = (value < 0) ? 0 : (value > 100) ? 100 : value;
    brushless_duty.duty = value;
    int err = ioctl(fd, BRUSHLESS_SET_DUTY, &brushless_duty);
    if(err < 0) {
        perror("Failed to set brushless duty\r\n");
        return;
    }
    duty = value;
}

/*******************************************************************
 * @brief       获取当前占空比
 * 
 * @return      返回私有数据duty(该数据在set_duty成功是更新)
 * 
 * @example     //打印占空比
 *              std::cout << brushless.get_duty() << std::endl;
 ******************************************************************/
int Brushless::get_duty(void)
{
    return duty;
}
