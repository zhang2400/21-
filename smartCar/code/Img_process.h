#ifndef __IMG_PROCESS_H_
#define __IMG_PROCESS_H_


#include "parameter.h"
#include "utils.h"

#define EDGELINE_MAX     (100)
#define img_with         (160)
#define img_high         (120)
#define PI               3.14159265358979f
#define sample_dist       0.02f
#define angle_dist        0.2f
#define pixel_per_meter     102.0f//根据实际情况调整，100像素/米表示每米对应100像素
using namespace cv;
#define AT_IMAGE_CLIP(img, x, y)     AT_IMAGE(img, clip(x, 0, img.cols-1), clip(y, 0, img.rows-1))

typedef struct Edge_Line {
    int line[EDGELINE_MAX][2];
    int len;
    float angle[EDGELINE_MAX];
    float angle_max[EDGELINE_MAX];
    float center_line[EDGELINE_MAX][2];

}Edge_Line;


extern Edge_Line left_line;
extern Edge_Line right_line;
extern Edge_Line mid_line;
extern int Ypt0_rpts0s_id, Ypt1_rpts1s_id, Lpt0_rpts0s_id, Lpt1_rpts1s_id;//左右边线的Y点和L点在rpts0s和rpts1s中的id
extern bool Ypt0_found, Ypt1_found, Lpt0_found, Lpt1_found, is_straight0, is_straight1;//是否找到左右边线的Y点和L点,以及是否为长直道


void findline_righthand_adaptive(Mat img, int x, int y, int pts[][2], int *num);
void findline_lefthand_adaptive(Mat img, int x, int y, int pts[][2], int *num);
void find_leftbase(Mat img, int *x, int *y);
void find_rightbase(Mat img, int *x, int *y);
// void blur_points(float pts_in[][2], int num, float pts_out[][2], int kernel)//边线点平滑，kernel为平滑的窗口大小
void blur_points(float pts_in[][2], int num, float pts_out[][2], int kernel);
// void local_angle_points_int(int pts_in[][2], int num, int angle_out[], int dist);//计算局部角度变化率，dist为计算角度变化率的点距（整数，单位为像素）
void local_angle_points(float pts_in[][2], int num, float angle_out[], int dist);
// void nms_angle_int_optimized(int angle_in[], int num, int angle_out[], int kernel);//局部角度变化率非极大抑制，kernel为非极大抑制的窗口大小
void nms_angle(float angle_in[], int num, float angle_out[], int kernel);
// void track_leftline_int_practical(int pts_in[][2], int num, int pts_out[][2], int approx_num, float dist);//根据边线点计算中线点，approx_num为计算切线的窗口大小，dist为偏移距离（浮点数，赛道宽度的一半）
void track_leftline(float pts_in[][2], int num, float pts_out[][2], int approx_num, float dist);
// void track_rightline_int_practical(int pts_in[][2], int num, int pts_out[][2], int approx_num, float dist);//根据边线点计算中线点，approx_num为计算切线的窗口大小，dist为偏移距离（浮点数，赛道宽度的一半）
void track_rightline(float pts_in[][2], int num, float pts_out[][2], int approx_num, float dist);
void find_corners();//识别Y,L拐点
// void resample_points(int pts_in[][2], int num1, int pts_out[][2], int *num2, float dist);//点集等距采样，dist为等距采样的距离（浮点数，单位为米）
void resample_points(float pts_in[][2], int num1, float pts_out[][2], int *num2, float dist);

#endif