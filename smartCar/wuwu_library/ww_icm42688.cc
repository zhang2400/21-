#include "ww_icm42688.h"

enum icm42688_afs
{
    ICM42688_AFS_16G,// default
    ICM42688_AFS_8G,
    ICM42688_AFS_4G,
    ICM42688_AFS_2G,
    NUM_ICM42688__AFS
};
enum icm42688_aodr
{
    ICM42688_AODR_32000HZ,// default
    ICM42688_AODR_16000HZ,
    ICM42688_AODR_8000HZ,
    ICM42688_AODR_4000HZ,
    ICM42688_AODR_2000HZ,
    ICM42688_AODR_1000HZ,
    ICM42688_AODR_200HZ,
    ICM42688_AODR_100HZ,
    ICM42688_AODR_50HZ,
    ICM42688_AODR_25HZ,
    ICM42688_AODR_12_5HZ,
    ICM42688_AODR_6_25HZ,
    ICM42688_AODR_3_125HZ,
    ICM42688_AODR_1_5625HZ,
    ICM42688_AODR_500HZ,
    NUM_ICM42688_AODR
};

enum icm42688_gfs
{
    ICM42688_GFS_2000DPS,// default
    ICM42688_GFS_1000DPS,
    ICM42688_GFS_500DPS,
    ICM42688_GFS_250DPS,
    ICM42688_GFS_125DPS,
    ICM42688_GFS_62_5DPS,
    ICM42688_GFS_31_25DPS,
    ICM42688_GFS_15_625DPS,
    NUM_ICM42688_GFS
};
enum icm42688_godr
{
    ICM42688_GODR_32000HZ,// default
    ICM42688_GODR_16000HZ,
    ICM42688_GODR_8000HZ,
    ICM42688_GODR_4000HZ,
    ICM42688_GODR_2000HZ,
    ICM42688_GODR_1000HZ,
    ICM42688_GODR_200HZ,
    ICM42688_GODR_100HZ,
    ICM42688_GODR_50HZ,
    ICM42688_GODR_25HZ,
    ICM42688_GODR_12_5HZ,
    ICM42688_GODR_X0HZ,
    ICM42688_GODR_X1HZ,
    ICM42688_GODR_X2HZ,
    ICM42688_GODR_500HZ,
    NUM_ICM42688_GODR
};

ICM42688::ICM42688(void)
    : fd(-1)
    , gyro_x(0), gyro_z(0), gyro_y(0)
    , accel_x(0), accel_y(0), accel_z(0)
    , syn_gyro_x(0), syn_gyro_y(0), syn_gyro_z(0)
    , syn_accel_x(0), syn_accel_y(0), syn_accel_z(0)
    
{
    //初始化互斥锁
    pthread_mutex_init(&data_mutex, NULL);

    icm42688_afs afs = ICM42688_AFS_16G;                //内核驱动已经按最高性能设置请勿更改
    icm42688_gfs gfs = ICM42688_GFS_2000DPS;            //内核驱动已经按最高性能设置请勿更改
    switch(afs)
    {
    case ICM42688_AFS_2G:
        icm42688_acc_inv = 2 / 32768.0f;                // 加速度计量程为:±2g
        break;
    case ICM42688_AFS_4G:
        icm42688_acc_inv = 4 / 32768.0f;                // 加速度计量程为:±4g
        break;
    case ICM42688_AFS_8G:
        icm42688_acc_inv = 8 / 32768.0f;                // 加速度计量程为:±8g
        break;
    case ICM42688_AFS_16G:
        icm42688_acc_inv = 16 / 32768.0f;               // 加速度计量程为:±16g
        break;
    default:
        icm42688_acc_inv = 1;                           // 不转化为实际数据
        break;
    }
    switch(gfs)
    {
    case ICM42688_GFS_15_625DPS:
        icm42688_gyro_inv = 15.625f / 32768.0f;         // 陀螺仪量程为:±15.625dps
        break;
    case ICM42688_GFS_31_25DPS:
        icm42688_gyro_inv = 31.25f / 32768.0f;          // 陀螺仪量程为:±31.25dps
        break;
    case ICM42688_GFS_62_5DPS:
        icm42688_gyro_inv = 62.5f / 32768.0f;           // 陀螺仪量程为:±62.5dps
        break;
    case ICM42688_GFS_125DPS:
        icm42688_gyro_inv = 125.0f / 32768.0f;          // 陀螺仪量程为:±125dps
        break;
    case ICM42688_GFS_250DPS:
        icm42688_gyro_inv = 250.0f / 32768.0f;          // 陀螺仪量程为:±250dps
        break;
    case ICM42688_GFS_500DPS:
        icm42688_gyro_inv = 500.0f / 32768.0f;          // 陀螺仪量程为:±500dps
        break;
    case ICM42688_GFS_1000DPS:
        icm42688_gyro_inv = 1000.0f / 32768.0f;         // 陀螺仪量程为:±1000dps
        break;
    case ICM42688_GFS_2000DPS:
        icm42688_gyro_inv = 2000.0f / 32768.0f;         // 陀螺仪量程为:±2000dps
        break;
    default:
        icm42688_gyro_inv = 1;                          // 不转化为实际数据
        break;
    }
}

ICM42688::~ICM42688(void)
{
    pthread_mutex_destroy(&data_mutex);
    if(fd < 0) return;
    close(fd);
}

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
int ICM42688::icm42688_init(void)
{
    fd = open(ICM42688_DEVICE, O_RDWR);
    if (fd < 0) {
        std::cerr << "打开icm42688失败!!!" << std::endl;
        return -1;
    } else {
        std::cout << "icm42688初始化完毕" << std::endl;
    }
    return 0;
}

/*******************************************************************
 * @brief       更新加速度计数据
 * 
 * @example     icm42688.upDataAcc();
 * 
 * @note        更新内部加速度计数据并转换为实际数据(单位为 g(m/s^2))
 ******************************************************************/
void ICM42688::upDataAcc(void)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct icm42688_accel_data data;
    ioctl(fd, ICM42688_GET_ACCEL, &data);
    float accel_x_temp = data.x * icm42688_acc_inv;
    float accel_y_temp = data.y * icm42688_acc_inv;
    float accel_z_temp = data.z * icm42688_acc_inv;

    //用于线程同步
    pthread_mutex_lock(&data_mutex);
    accel_x = accel_x_temp;
    accel_y = accel_y_temp;
    accel_z = accel_z_temp;
    pthread_mutex_unlock(&data_mutex);
}

/*******************************************************************
 * @brief       更新陀螺仪数据
 * 
 * @example     icm42688.upDataGyro();
 * 
 * @note        更新内部陀螺仪数据并转换为实际数据(单位为 °/s)
 ******************************************************************/
void ICM42688::upDataGyro(void)
{
    if(fd < 0) {
        std::cout <<"设备未初始化!!!" << std::endl;
        return;
    }

    struct icm42688_gyro_data data;
    ioctl(fd, ICM42688_GET_GYRO, &data);
    float gyro_x_temp = data.x * icm42688_gyro_inv;
    float gyro_y_temp = data.y * icm42688_gyro_inv;
    float gyro_z_temp = data.z * icm42688_gyro_inv;

    //用于线程同步
    pthread_mutex_lock(&data_mutex);
    gyro_x = gyro_x_temp;
    gyro_y = gyro_y_temp;
    gyro_z = gyro_z_temp;
    pthread_mutex_unlock(&data_mutex);
}

/*******************************************************************
 * @brief       线程同步操作
 * 
 * @example     icm42688.thread_syn();
 * 
 * @note        在使用syn_xxx数据前, 调用此函数, 用于线程同步, 保证数据安全
 ******************************************************************/
void ICM42688::thread_syn(void)
{
    //用于线程同步
    pthread_mutex_lock(&data_mutex);
    syn_gyro_x = gyro_x;
    syn_gyro_y = gyro_y;
    syn_gyro_z = gyro_z;

    syn_accel_x = accel_x;
    syn_accel_y = accel_y;
    syn_accel_z = accel_z;
    pthread_mutex_unlock(&data_mutex);
}
