#ifndef __ROT_H_
#define __ROT_H_

#include "headfile.h"
using namespace cv;
extern const float mapx[120][160];
extern const float mapy[120][160];
float Xrot_point(uint8_t x,uint8_t y);
float Yrot_point(uint8_t x, uint8_t y);
float inv_rot_x(float x, float y);
float inv_rot_y(float x, float y);
float XShowRotimg(uint8_t x,uint8_t y);
float YShowRotimg(uint8_t x,uint8_t y);
void nitoushi_fast(const Mat& src, Mat& dst);
bool map_inv(float pt0[2], int pt1[2]);
#endif