#ifndef __WUWU_ICM42688_H_
#define __WUWU_ICM42688_H_

#include "headfile.h"

#define ICM42688_DEVICE                 "/dev/wuwu_icm42688"

class ICM42688 {
public:
/**
 * @brief       未进行线程同步的数据
 * 
 * @note        如果读取数据和使用数据在同一线程,直接使用下面的即可
 */
    float gyro_x;           // 陀螺仪 x         (单位为 °/s)
    float gyro_y;           // 陀螺仪 y         (单位为 °/s)
    float gyro_z;           // 陀螺仪 z         (单位为 °/s)

    float accel_x;          // 加速度计 x       (单位为 g(m/s^2))
    float accel_y;          // 加速度计 y       (单位为 g(m/s^2))
    float accel_z;          // 加速度计 z       (单位为 g(m/s^2))

/**
 * @brief       线程同步后的数据
 * 
 * @note        如果读取不在此线程, 则先需要调用函数 thread_syn();
 *              再使用以下数据, 否则数据为空
 */
    float syn_gyro_x;       // 陀螺仪 x         (单位为 °/s)
    float syn_gyro_y;       // 陀螺仪 y         (单位为 °/s)
    float syn_gyro_z;       // 陀螺仪 z         (单位为 °/s)
    float syn_accel_x;      // 加速度计 x       (单位为 g(m/s^2))
    float syn_accel_y;      // 加速度计 y       (单位为 g(m/s^2))
    float syn_accel_z;      // 加速度计 z       (单位为 g(m/s^2))

    ICM42688(void);
    ~ICM42688(void);
/*******************************************************************
 * @brief       初始化陀螺仪
 * 
 * @return      返回初始化状态
 * @retval      0               初始化成功
 * @retval      -1              初始化失败
 * 
 * @example     //初始化ICM42688
 *              if(icm42688.icm42688_init() < 0) {
 *                  return -1;
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/
    int icm42688_init(void);

/*******************************************************************
 * @brief       更新加速度计数据
 * 
 * @example     icm42688.upDataAcc();
 * 
 * @note        更新内部加速度计数据并转换为实际数据(单位为 g(m/s^2))
 ******************************************************************/
    void upDataAcc(void);

/*******************************************************************
 * @brief       更新陀螺仪数据
 * 
 * @example     icm42688.upDataGyro();
 * 
 * @note        更新内部陀螺仪数据并转换为实际数据(单位为 °/s)
 ******************************************************************/
    void upDataGyro(void);

/*******************************************************************
 * @brief       线程同步操作
 * 
 * @example     icm42688.thread_syn();
 * 
 * @note        在使用syn_xxx数据前, 调用此函数, 用于线程同步, 保证数据安全
 ******************************************************************/
    void thread_syn(void);

private:
    // 设备文件描述符
    int fd;
    // 用于线程同步—互斥锁
    pthread_mutex_t data_mutex;

    // 数据转换为实际物理数据的转换系数
    float icm42688_acc_inv = 1;
    float icm42688_gyro_inv = 1;
    
};

#endif
