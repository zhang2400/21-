#ifndef __HEADFILE_H_
#define __HEADFILE_H_

/* 第三方库 */
#include <opencv2/opencv.hpp>
#include <json/json.h>
#include <net.h>

/* 系统库 */
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>  
#include <unistd.h>     
#include <iostream>
#include <chrono>
#include <pthread.h>
#include <time.h>
#include <fstream>
#include <cmath>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <cstdint>
#include <thread>
#include <mutex>
#include <linux/input.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <cstring>

/* wuwu库 */
#include "ww_ioctl.h"
#include "ww_key.h"
#include "ww_brushless.h"
#include "ww_icm42688.h"
#include "ww_motor.h"
#include "ww_buzzer.h"
#include "ww_lcd.h"
#include "ww_vl53l0x.h"
#include "ww_timerThread.h"
#include "ww_camera_server.h"
#include "ww_tcp_client.h"
#include "font.h"
#include "music.h"

// /* 个人库 */
// #include "Img_process.h"
// #include "PID.h"
// #include "garage.h"
// #include "utils.h"
// #include "cross.h"


/* 全局变量应用 */
enum track_type_e {
    TRACK_LEFT,
    TRACK_RIGHT,
};
extern enum track_type_e track_type;




#endif
