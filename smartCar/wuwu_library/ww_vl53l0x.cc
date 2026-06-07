#include "ww_vl53l0x.h"

VL53L0X::VL53L0X(void)
    : fd(-1)
    , vl53l0x_distance_mm(8192)
    , syn_vl53l0x_distance_mm(8192)
{
    pthread_mutex_init(&data_mutex, NULL);
}

VL53L0X::~VL53L0X(void)
{
    pthread_mutex_destroy(&data_mutex);
    if(fd < 0) return;
    close(fd);
}

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
int VL53L0X::tof_init(void)
{
    fd = open(DEVICE_TOF_ADDR, O_WRONLY);
    if(fd < 0) {
        perror("Error open tof\r\n");
        return -1;
    } else {
        std::cout << "TOF初始化成功" << std::endl;
    }
    return 0;
}

/*******************************************************************
 * @brief       更新TOF数据
 * 
 * @example     tof.upData();
 ******************************************************************/
void VL53L0X::upData(void)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    uint16_t data;
    if(ioctl(fd, VL53L0X_GET_DATA, &data)) {
        perror("Failed to get tof data\r\n");
        return;
    }
    vl53l0x_distance_mm = data;
}

/*******************************************************************
 * @brief       线程同步操作
 * 
 * @example     tof.thread_syn();
 * 
 * @note        在使用syn_xxx数据前, 调用此函数, 用于线程同步, 保证数据安全
 ******************************************************************/
void VL53L0X::thread_syn(void)
{
    pthread_mutex_lock(&data_mutex);
    syn_vl53l0x_distance_mm = vl53l0x_distance_mm;
    pthread_mutex_unlock(&data_mutex);
}
