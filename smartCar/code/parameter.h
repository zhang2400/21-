// zebra_cross.h
#ifndef PARAMETER_H
#define PARAMETER_H

#include "headfile.h"


using namespace cv;
//cross.c中使用的全局变量和常量
#define FAR_POINTS_MAX_LEN 100
extern float aim_distance;//预锚点距离，单位米
extern int begin_x;// 起始点距离图像中心的左右偏移量，单位像素
extern int begin_y;// 起始点距离图像底部的上下偏移量，单位像素
extern float line_blur_kernel;//边线滤波核大小，单位像素
extern bool far_Lpt0_found, far_Lpt1_found;
extern int far_Lpt0_rpts0s_id, far_Lpt1_rpts1s_id;

extern int far_ipts0[FAR_POINTS_MAX_LEN][2];//十字远线left边线点
extern int far_ipts1[FAR_POINTS_MAX_LEN][2];//十字远线right边线点
extern int far_ipts0_num, far_ipts1_num;//十字远线边线点数量

extern float far_rpts0[FAR_POINTS_MAX_LEN][2];//左侧原始追踪点
extern float far_rpts1[FAR_POINTS_MAX_LEN][2];//右侧原始追踪点
extern int far_rpts0_num, far_rpts1_num;//左侧原始追踪点数量，右侧原始追踪点数量

extern float far_rpts0b[FAR_POINTS_MAX_LEN][2];//左侧滤波后点
extern float far_rpts1b[FAR_POINTS_MAX_LEN][2];//右侧滤波后点
extern int far_rpts0b_num, far_rpts1b_num;//左侧滤波后点数量，右侧滤波后点数量

extern float far_rpts0s[FAR_POINTS_MAX_LEN][2];//左侧等距采样点
extern float far_rpts1s[FAR_POINTS_MAX_LEN][2];//右侧等距采样点
extern int far_rpts0s_num, far_rpts1s_num;//左侧等距采样点数量，右侧等距采样点数量
extern float far_rpts0a[FAR_POINTS_MAX_LEN];//左侧每个点的角度变化率
extern float far_rpts1a[FAR_POINTS_MAX_LEN];//右侧每个点的角度变化率
extern int far_rpts0a_num, far_rpts1a_num;

extern float far_rpts0an[FAR_POINTS_MAX_LEN];//抑制后的角度变化率（保留局部最大值）
extern float far_rpts1an[FAR_POINTS_MAX_LEN];
extern int far_rpts0an_num, far_rpts1an_num;


// main.cc中使用的全局变量
#define ROAD_WIDTH 0.43f//赛道宽度
extern Motor motor;
extern Mat img;
extern Mat img_raw;
extern Mat img_mb;
extern Mat bird_img; 
extern Mat img_bin;
extern float angle;// 当前转角，单位度
extern int left_b[FAR_POINTS_MAX_LEN][2];
extern int left_bnum;
extern int right_b[FAR_POINTS_MAX_LEN][2];
extern int right_bnum;
extern int left_rp[FAR_POINTS_MAX_LEN][2];
extern int left_rpnum;
extern int right_rp[FAR_POINTS_MAX_LEN][2];
extern int right_rpnum;
extern float mid_ui[FAR_POINTS_MAX_LEN][2];
extern int mid_uinum;

extern float rpts0[FAR_POINTS_MAX_LEN][2];
extern int rpts0_num;
extern float rpts1[FAR_POINTS_MAX_LEN][2];
extern int rpts1_num;
extern float rpts0b[FAR_POINTS_MAX_LEN][2];
extern int rpts0b_num;
extern float rpts1b[FAR_POINTS_MAX_LEN][2];
extern int rpts1b_num;
extern float rpts0s[FAR_POINTS_MAX_LEN][2];
extern int rpts0s_num;
extern float rpts1s[FAR_POINTS_MAX_LEN][2];
extern int rpts1s_num;
extern float rptsc0[FAR_POINTS_MAX_LEN][2];
extern int rptsc0_num;
extern float rptsc1[FAR_POINTS_MAX_LEN][2];
extern int rptsc1_num;
extern int rpts0a_num, rpts1a_num;
extern int rpts0an_num, rpts1an_num;
#endif // PARAMETER_H