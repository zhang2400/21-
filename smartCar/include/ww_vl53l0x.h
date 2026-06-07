#ifndef __WUWU_VL53L0X_H_
#define __WUWU_VL53L0X_H_

#include "headfile.h"

#define DEVICE_TOF_ADDR         "/dev/wuwu_tof"

class VL53L0X
{
public:
/**
 * @brief       未进行线程同步的数据
 * 
 * @note        如果读取数据和使用数据在同一线程,直接使用下面的即可
 */
    uint16_t vl53l0x_distance_mm;

/**
 * @brief       线程同步后的数据
 * 
 * @note        如果读取不在此线程, 则先需要调用函数 thread_syn();
 *              再使用以下数据, 否则数据为空
 */
    uint16_t syn_vl53l0x_distance_mm;

    VL53L0X(void);
    ~VL53L0X(void);
/*******************************************************************
 * @brief       TOF初始化
 * 
 * @return      返回初始化状态
 * @retval      0               初始化成功
 * @retval      -1              初始化失败
 * 
 * @example     //蜂鸣器初始化
 *              if(tof.tof_init() < 0) {
 *                  return -1;             
 *              }
 * 
 * @note        不使用此函数直接使用下面函数会报错
 ******************************************************************/ 
    int tof_init(void);

/*******************************************************************
 * @brief       更新TOF数据
 * 
 * @example     tof.upData();
 ******************************************************************/
    void upData(void);

/*******************************************************************
 * @brief       线程同步操作
 * 
 * @example     tof.thread_syn();
 * 
 * @note        在使用syn_xxx数据前, 调用此函数, 用于线程同步, 保证数据安全
 ******************************************************************/
    void thread_syn(void);

private:
    // 设备文件描述符
    int fd;
    // 用于线程同步—互斥锁
    pthread_mutex_t data_mutex;
};


#endif