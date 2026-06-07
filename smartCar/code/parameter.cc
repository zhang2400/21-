#include "parameter.h"

float aim_distance = 0.01;//预锚点距离，单位米
int begin_x = 30;// 起始点距离图像中心的左右偏移量
int begin_y = 110;// 起始点距离图像底部的上下偏移
float line_blur_kernel = 5.0f;//边线滤波核大小，单位像素

// main.cc中使用的全局变量
int left_b[FAR_POINTS_MAX_LEN][2];
int left_bnum;
int right_b[FAR_POINTS_MAX_LEN][2];
int right_bnum;
int left_rp[FAR_POINTS_MAX_LEN][2];
int left_rpnum;
int right_rp[FAR_POINTS_MAX_LEN][2];
int right_rpnum;
float mid_ui[FAR_POINTS_MAX_LEN][2];
int mid_uinum;

float rpts0[FAR_POINTS_MAX_LEN][2];
int rpts0_num;
float rpts1[FAR_POINTS_MAX_LEN][2];
int rpts1_num;
float rpts0b[FAR_POINTS_MAX_LEN][2];
int rpts0b_num;
float rpts1b[FAR_POINTS_MAX_LEN][2];
int rpts1b_num;
float rpts0s[FAR_POINTS_MAX_LEN][2];
int rpts0s_num;
float rpts1s[FAR_POINTS_MAX_LEN][2];
int rpts1s_num;
float rptsc0[FAR_POINTS_MAX_LEN][2];
int rptsc0_num;
float rptsc1[FAR_POINTS_MAX_LEN][2];
int rptsc1_num;
int rpts0a_num, rpts1a_num;
int rpts0an_num, rpts1an_num;

