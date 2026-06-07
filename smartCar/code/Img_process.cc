#include "Img_process.h"

using namespace cv;
Edge_Line left_line = {0};
Edge_Line right_line = {0};
Edge_Line mid_line = {0};
int Ypt0_rpts0s_id, Ypt1_rpts1s_id, Lpt0_rpts0s_id, Lpt1_rpts1s_id;
bool Ypt0_found, Ypt1_found, Lpt0_found, Lpt1_found, is_straight0, is_straight1;

const int dir_front[4][2]= {{0,  -1},
                             {1,  0},
                             {0,  1},
                             {-1, 0}};

const int dir_frontleft[4][2] = {{-1, -1},
                                 {1,  -1},
                                 {1,  1},
                                 {-1, 1}};

const int dir_frontright[4][2] = {{1,  -1},
                                  {1,  1},
                                  {-1, 1},
                                  {-1, -1}};


// 左手迷宫巡线
void findline_lefthand_adaptive(Mat img, int x, int y, int pts[][2], int *num)
 {
   
    int half = 1;
    uint8_t *ptr;
    int step = 0, dir = 0, turn = 0;

    while (step < *num && half < x && x < img.cols - half - 1 && half < y && y < img.rows - half - 1 && turn < 4) {
        int local_thres = 125;

        ptr = img.ptr(y);
        int current_value = ptr[x];

        ptr = img.ptr(y + dir_front[dir][1]);
        int front_value = ptr[x + dir_front[dir][0]];
        
        ptr = img.ptr(y + dir_frontleft[dir][1]);
        int frontleft_value = ptr[x + dir_frontleft[dir][0]];
        
        if (front_value < local_thres) {
            dir = (dir + 1) % 4;
            turn++;
        } else if (frontleft_value < local_thres) {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        } else {
            x += dir_frontleft[dir][0];
            y += dir_frontleft[dir][1];
            dir = (dir + 3) % 4;
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        }
    }
    *num = step;
}


void find_leftbase(Mat img, int *x, int *y)
{
    uint8_t *ptr =img.ptr(*y);
    for(int w = *x; w > 1; w--) {
        if(ptr[w] == 255 && ptr[w-1] == 0)
        {
            *x = w;
            return;
        }
    }
    *x = 1;
}

// 右手迷宫巡线
void findline_righthand_adaptive(Mat img, int x, int y, int pts[][2], int *num) {
   
    int half = 1;
    uint8_t *ptr;
    int step = 0, dir = 0, turn = 0;

    // // ===== 修改1：存储起始点 =====
    // pts[step][0] = x;
    // pts[step][1] = y;
    // step++;
    // // ===========================
    while (step < *num && half < x && x < img.cols - half - 1 && half < y && y < img.rows - half - 1 && turn < 4) 
    {
        int local_thres = 125;        

        ptr = img.ptr(y);
        int current_value = ptr[x];

        ptr = img.ptr(y + dir_front[dir][1]);
        int front_value = ptr[x + dir_front[dir][0]];
        
        ptr = img.ptr(y + dir_frontright[dir][1]);
        int frontright_value = ptr[x + dir_frontright[dir][0]];
       
        if (front_value < local_thres) {
            dir = (dir + 3) % 4;
            turn++;
        } else if (frontright_value < local_thres) {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        } else {
            x += dir_frontright[dir][0];
            y += dir_frontright[dir][1];
            dir = (dir + 1) % 4;
            pts[step][0] = x;
            pts[step][1] = y;
            step++;
            turn = 0;
        }
    }
    *num = step;
}
void find_rightbase(Mat img, int *x, int *y)
{
    uint8_t *ptr =img.ptr(*y);
    for(int w = *x; w < 155; w++) {
        if(ptr[w] == 255 && ptr[w+1] == 0)
        {
            *x = w;
            return;
        }
    }
    *x = 1;
}


// // 修复版点集三角滤波
// void blur_points(int pts_in[][2], int num, int pts_out[][2], int kernel) {
//     if (num < kernel) {
//         // 如果点数少于核大小，直接复制，不滤波
//         for (int i = 0; i < num; i++) {
//             pts_out[i][0] = pts_in[i][0];
//             pts_out[i][1] = pts_in[i][1];
//         }
//         return;
//     }
    
//     assert(kernel % 2 == 1);
//     int half = kernel / 2;
    
//     // 预计算权重
//     float weights[50];  // 假设最大kernel为99
//     float weight_sum = 0;
//     for (int j = -half; j <= half; j++) {
//         weights[j + half] = half + 1 - abs(j);
//         weight_sum += weights[j + half];
//     }
    
//     for (int i = 0; i < num; i++) {
//         float sum_x = 0, sum_y = 0;
        
//         for (int j = -half; j <= half; j++) {
//             int idx = clip(i + j, 0, num - 1);
//             float w = weights[j + half];
//             sum_x += pts_in[idx][0] * w;
//             sum_y += pts_in[idx][1] * w;
//         }
        
//         // 使用四舍五入而不是截断
//         pts_out[i][0] = (int)round(sum_x / weight_sum);
//         pts_out[i][1] = (int)round(sum_y / weight_sum);
//     }
// }
void blur_points(float pts_in[][2], int num, float pts_out[][2], int kernel) {
    assert(kernel % 2 == 1);
    int half = kernel / 2;
    for (int i = 0; i < num; i++) {
        pts_out[i][0] = pts_out[i][1] = 0;
        for (int j = -half; j <= half; j++) {
            pts_out[i][0] += pts_in[clip(i + j, 0, num - 1)][0] * (half + 1 - abs(j));
            pts_out[i][1] += pts_in[clip(i + j, 0, num - 1)][1] * (half + 1 - abs(j));
        }
        pts_out[i][0] /= (2 * half + 2) * (half + 1) / 2;
        pts_out[i][1] /= (2 * half + 2) * (half + 1) / 2;
    }
}

// 点集局部角度变化率
// 点集局部角度变化率 - 输出为整数角度（度）
// 输入：整数坐标点
// 输出：角度值，范围-180~180
// void local_angle_points_int(int pts_in[][2], int num, int angle_out[], int dist) {
//     // 预计算常量
    
//     const float RAD_TO_DEG = 180.0f / 3.14159265f;
    
//     for (int i = 0; i < num; i++) {
//         // 边界点角度为0（与原版本一致：只排除首尾点）
//         if (i <= 0 || i >= num - 1) {
//             angle_out[i] = 0;
//             continue;
//         }
        
//         // 获取前后点坐标（使用clip处理边界）
//         int idx_prev = (i - dist < 0) ? 0 : i - dist;
//         int idx_next = (i + dist >= num) ? num - 1 : i + dist;
        
//         int dx1 = pts_in[i][0] - pts_in[idx_prev][0];
//         int dy1 = pts_in[i][1] - pts_in[idx_prev][1];
//         int dx2 = pts_in[idx_next][0] - pts_in[i][0];
//         int dy2 = pts_in[idx_next][1] - pts_in[i][1];
        
//         // 计算向量长度
//         float len1 = sqrtf((float)(dx1 * dx1 + dy1 * dy1));
//         float len2 = sqrtf((float)(dx2 * dx2 + dy2 * dy2));
        
//         // 防止除零
//         if (len1 < 0.5f || len2 < 0.5f) {
//             angle_out[i] = 0;
//             continue;
//         }
        
//         // 计算单位向量分量（与原版本一致）
//         float c1 = (float)dx1 / len1;
//         float s1 = (float)dy1 / len1;
//         float c2 = (float)dx2 / len2;
//         float s2 = (float)dy2 / len2;
        
//         // 使用与原版本相同的公式计算角度
//         float angle_rad = atan2f(c1 * s2 - c2 * s1, c2 * c1 + s2 * s1);
        
//         // 转换为角度并四舍五入
//         angle_out[i] = (int)(angle_rad * RAD_TO_DEG + 0.5f);
        
//         // 确保范围
//         if (angle_out[i] > 180) angle_out[i] = 180;
//         if (angle_out[i] < -180) angle_out[i] = -180;
//     }
// }

void local_angle_points(float pts_in[][2], int num, float angle_out[], int dist) {
    for (int i = 0; i < num; i++) {
        if (i <= 0 || i >= num - 1) {
            angle_out[i] = 0;
            continue;
        }
        float dx1 = pts_in[i][0] - pts_in[clip(i - dist, 0, num - 1)][0];
        float dy1 = pts_in[i][1] - pts_in[clip(i - dist, 0, num - 1)][1];
        float dn1 = sqrtf(dx1 * dx1 + dy1 * dy1);
        float dx2 = pts_in[clip(i + dist, 0, num - 1)][0] - pts_in[i][0];
        float dy2 = pts_in[clip(i + dist, 0, num - 1)][1] - pts_in[i][1];
        float dn2 = sqrtf(dx2 * dx2 + dy2 * dy2);
        float c1 = dx1 / dn1;
        float s1 = dy1 / dn1;
        float c2 = dx2 / dn2;
        float s2 = dy2 / dn2;
        angle_out[i] = atan2f(c1 * s2 - c2 * s1, c2 * c1 + s2 * s1);
    }
}
// 角度变化率非极大抑制
// 输入：整数角度（度），范围-180~180
// 输出：保留峰值的整数角度
// void nms_angle_int_optimized(int angle_in[], int num, int angle_out[], int kernel) {
//     assert(kernel % 2 == 1);
//     int half = kernel / 2;
    
//     // 初始化输出为输入值（与原版本一致）
//     memcpy(angle_out, angle_in, num * sizeof(int));
    
//     for (int i = 0; i < num; i++) {
//         // 跳过0值（优化，与原版本逻辑一致）
//         if (angle_out[i] == 0) continue;
        
//         int current_abs = abs(angle_out[i]);
        
//         // 检查邻域
//         for (int j = -half; j <= half; j++) {
//             if (j == 0) continue;
            
//             int idx = (i + j < 0) ? 0 : ((i + j >= num) ? num - 1 : i + j);
//             int neighbor_abs = abs(angle_in[idx]);
            
//             // 与原版本一致：只要邻域内有更大幅值的点，当前点就清零
//             if (neighbor_abs > current_abs) {
//                 angle_out[i] = 0;
//                 break;
//             }
//         }
//     }
    
// }
void nms_angle(float angle_in[], int num, float angle_out[], int kernel) {
    assert(kernel % 2 == 1);
    int half = kernel / 2;
    for (int i = 0; i < num; i++) {
        angle_out[i] = angle_in[i];
        for (int j = -half; j <= half; j++) {
            if (fabs(angle_in[clip(i + j, 0, num - 1)]) > fabs(angle_out[i])) {
                angle_out[i] = 0;
                break;
            }
        }
    }
}

// 左边线跟踪中线 - 实用整数版
// 输入输出都是整数坐标，内部使用浮点保证精度
// void track_leftline_int_practical(int pts_in[][2], int num, int pts_out[][2], int approx_num, float dist) {
    
//     for (int i = 0; i < num; i++) {
//         int prev_idx = clip(i - approx_num, 0, num - 1);
//         int next_idx = clip(i + approx_num, 0, num - 1);
        
//         float dx = (float)(pts_in[next_idx][0] - pts_in[prev_idx][0]);
//         float dy = (float)(pts_in[next_idx][1] - pts_in[prev_idx][1]);
//         float dn = sqrtf(dx * dx + dy * dy);
        
//         if (dn < 0.001f) {
//             pts_out[i][0] = pts_in[i][0];
//             pts_out[i][1] = pts_in[i][1];
//             continue;
//         }
        
//         dx /= dn;
//         dy /= dn;
        
//         // 左边线向右偏移（法线方向相反）
//         float new_x = pts_in[i][0] - dy * dist;
//         float new_y = pts_in[i][1] + dx * dist;
        
//         pts_out[i][0] = (int)(new_x + 0.5f);
//         pts_out[i][1] = (int)(new_y + 0.5f);
//     }
// }

void track_leftline(float pts_in[][2], int num, float pts_out[][2], int approx_num, float dist) {
    for (int i = 0; i < num; i++) {
        float dx = pts_in[clip(i + approx_num, 0, num - 1)][0] - pts_in[clip(i - approx_num, 0, num - 1)][0];
        float dy = pts_in[clip(i + approx_num, 0, num - 1)][1] - pts_in[clip(i - approx_num, 0, num - 1)][1];
        float dn = sqrt(dx * dx + dy * dy);
        dx /= dn;
        dy /= dn;
        pts_out[i][0] = pts_in[i][0] - dy * dist;
        pts_out[i][1] = pts_in[i][1] + dx * dist;
    }
}
// 右边线跟踪中线 - 实用整数版
// 输入：pts_in - 整数坐标点集（右边线）
//      num - 点数
//      approx_num - 计算切线的窗口大小
//      dist - 偏移距离（浮点数，赛道宽度的一半）
// 输出：pts_out - 偏移后的整数坐标点集（中线）
// void track_rightline_int_practical(int pts_in[][2], int num, int pts_out[][2], int approx_num, float dist) {
//     for (int i = 0; i < num; i++) {
//         // 使用clip处理边界，所有点都计算偏移
//         int prev_idx = clip(i - approx_num, 0, num - 1);
//         int next_idx = clip(i + approx_num, 0, num - 1);
        
//         // 计算切线向量
//         float dx = (float)(pts_in[next_idx][0] - pts_in[prev_idx][0]);
//         float dy = (float)(pts_in[next_idx][1] - pts_in[prev_idx][1]);
//         float dn = sqrtf(dx * dx + dy * dy);
        
//         // 防止除零（浮点版没有，但加上更安全）
//         if (dn < 0.001f) {
//             pts_out[i][0] = pts_in[i][0];
//             pts_out[i][1] = pts_in[i][1];
//             continue;
//         }
        
//         // 归一化切线向量
//         dx /= dn;
//         dy /= dn;
        
//         // 计算偏移点（右边线向左偏移）
//         float new_x = pts_in[i][0] + dy * dist;
//         float new_y = pts_in[i][1] - dx * dist;
        
//         // 四舍五入为整数
//         pts_out[i][0] = (int)(new_x + 0.5f);
//         pts_out[i][1] = (int)(new_y + 0.5f);
//     }
    
// }

void track_rightline(float pts_in[][2], int num, float pts_out[][2], int approx_num, float dist) {
    for (int i = 0; i < num; i++) {
        float dx = pts_in[clip(i + approx_num, 0, num - 1)][0] - pts_in[clip(i - approx_num, 0, num - 1)][0];
        float dy = pts_in[clip(i + approx_num, 0, num - 1)][1] - pts_in[clip(i - approx_num, 0, num - 1)][1];
        float dn = sqrt(dx * dx + dy * dy);
        dx /= dn;
        dy /= dn;
        pts_out[i][0] = pts_in[i][0] + dy * dist;
        pts_out[i][1] = pts_in[i][1] - dx * dist;
    }
}

void find_corners() {
    // 识别Y,L拐点
    
    Ypt0_found = Ypt1_found = Lpt0_found = Lpt1_found = false;
    is_straight0 = rpts0s_num > 50;
    is_straight1 = rpts1s_num > 50;
    for (int i = 0; i < rpts0s_num; i++) {
        if (left_line.angle_max[i] == 0) continue;
        int im1 = clip(i - (int) round(angle_dist / sample_dist), 0, rpts0s_num - 1);//向前识别的点数
        int ip1 = clip(i + (int) round(angle_dist / sample_dist), 0, rpts0s_num - 1);
        float conf = fabs(left_line.angle[i] - (fabs(left_line.angle[im1]) + fabs(left_line.angle[ip1]))/2) ;
        
        // float conf = fabs(left_line.angle_max[i]);  // 直接使用峰值角度
        //Y角点阈值
        if (Ypt0_found == false && 30. / 180. * PI < conf && conf < 65. / 180. * PI && i < 0.8 / sample_dist) {
            Ypt0_rpts0s_id = i;
            Ypt0_found = true;
        }
        //L角点阈值
        if (Lpt0_found == false && 70. / 180. * PI < conf && conf < 140. / 180. * PI && i < 0.8 / sample_dist) {
            Lpt0_rpts0s_id = i;
            Lpt0_found = true;
        }
        //长直道阈值
        if (conf > 5. / 180. * PI && i < 1.0 / sample_dist) is_straight0 = false;
        if (Ypt0_found == true && Lpt0_found == true && is_straight0 == false) break;
    }
    for (int i = 0; i < rpts1s_num; i++) {
        if (right_line.angle_max[i] == 0) continue;
        int im1 = clip(i - (int) round(angle_dist / sample_dist), 0, rpts1s_num - 1);
        int ip1 = clip(i + (int) round(angle_dist / sample_dist), 0, rpts1s_num - 1);
        float conf = fabs(right_line.angle[i]) - (fabs(right_line.angle[im1]) + fabs(right_line.angle[ip1])) / 2;
        // float conf = fabs(right_line.angle_max[i]);  // 直接使用峰值角度
        if (Ypt1_found == false && 30. / 180. * PI < conf && conf < 65. / 180. * PI && i < 0.8 / sample_dist) {
            Ypt1_rpts1s_id = i;
            Ypt1_found = true;
        }
        if (Lpt1_found == false && 70. / 180. * PI < conf && conf < 140. / 180. * PI && i < 0.8 / sample_dist) {
            Lpt1_rpts1s_id = i;
            Lpt1_found = true;
        }

        if (conf > 5. / 180. * PI && i < 1.0 / sample_dist) is_straight1 = false;

        if (Ypt1_found == true && Lpt1_found == true && is_straight1 == false) break;
    }
    // Y点二次检查,依据两角点距离及角点后张开特性
    if (Ypt0_found && Ypt1_found) {
        float dx = rpts0s[Ypt0_rpts0s_id][0] - rpts1s[Ypt1_rpts1s_id][0];
        float dy = rpts0s[Ypt0_rpts0s_id][1] - rpts1s[Ypt1_rpts1s_id][1];
        float dn = sqrtf(dx * dx + dy * dy);
        if (fabs(dn - 0.45 * pixel_per_meter) < 0.15 * pixel_per_meter) {
            float dwx = rpts0s[clip(Ypt0_rpts0s_id + 50, 0, rpts0s_num - 1)][0] -
                        rpts1s[clip(Ypt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0];
            float dwy = rpts0s[clip(Ypt0_rpts0s_id + 50, 0, rpts0s_num - 1)][1] -
                        rpts1s[clip(Ypt1_rpts1s_id + 50, 0, rpts1s_num - 1)][1];
            float dwn = sqrtf(dwx * dwx + dwy * dwy);
            if (!(dwn > 0.7 * pixel_per_meter &&
                  rpts0s[clip(Ypt0_rpts0s_id + 50, 0, rpts0s_num- 1)][0] < rpts0s[Ypt0_rpts0s_id][0] &&
                  rpts1s[clip(Ypt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0] > rpts1s[Ypt1_rpts1s_id][0])) {
                Ypt0_found = Ypt1_found = false;
            }
        } else {
            Ypt0_found = Ypt1_found = false;
        }
    }
    // L点二次检查，车库模式不检查, 依据L角点距离及角点后张开特性
    if (1) {
        if (Lpt0_found && Lpt1_found) {
            float dx = rpts0s[Lpt0_rpts0s_id][0] - rpts1s[Lpt1_rpts1s_id][0];
            float dy = rpts0s[Lpt0_rpts0s_id][1] - rpts1s[Lpt1_rpts1s_id][1];
            float dn = sqrtf(dx * dx + dy * dy);
            if (fabs(dn - 0.45 * pixel_per_meter) < 0.15 * pixel_per_meter) {
                float dwx = rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num - 1)][0] -
                            rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0];
                float dwy = rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num - 1)][1] -
                            rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][1];
                float dwn = sqrtf(dwx * dwx + dwy * dwy);
                if (!(dwn > 0.7 * pixel_per_meter &&
                      rpts0s[clip(Lpt0_rpts0s_id + 50, 0, rpts0s_num- 1)][0] < rpts0s[Lpt0_rpts0s_id][0] &&
                      rpts1s[clip(Lpt1_rpts1s_id + 50, 0, rpts1s_num - 1)][0] > rpts1s[Lpt1_rpts1s_id][0])) {
                    Lpt0_found = Lpt1_found = false;
                }
            } else {
                Lpt0_found = Lpt1_found = false;
            }
        }
    }
}


// 点集等距采样  使走过的采样前折线段的距离为`dist`
// void resample_points(int pts_in[][2], int num1, int pts_out[][2], int *num2, float dist){
//     float remain = 0.f;
//     int len = 0;
//     for(int i=0; i<num1-1 && len < *num2; i++){
//         float x0 = (float)pts_in[i][0];
//         float y0 = (float)pts_in[i][1];
//         float dx = (float)(pts_in[i+1][0] - pts_in[i][0]);
//         float dy = (float)(pts_in[i+1][1] - pts_in[i][1]);
//         float dn = sqrt(dx*dx + dy*dy);
        
//         // 避免除零错误
//         if (dn < 1e-6f) continue;
        
//         dx /= dn;
//         dy /= dn;

//         while(remain < dn && len < *num2){
//             x0 += dx * remain;
//             y0 += dy * remain;
//             pts_out[len][0] = (int)roundf(x0);
//             pts_out[len][1] = (int)roundf(y0);
            
//             len++;
//             dn -= remain;
//             remain = dist;
//         }
//         remain -= dn;
//     }
//     *num2 = len;
// }
void resample_points(float pts_in[][2], int num1, float pts_out[][2], int *num2, float dist){
    float remain = 0.f;
    int len = 0;
    for(int i=0; i<num1-1 && len < *num2; i++){
        float x0 = pts_in[i][0];
        float y0 = pts_in[i][1];
        float dx = pts_in[i+1][0] - x0;
        float dy = pts_in[i+1][1] - y0;
        float dn = sqrt(dx*dx+dy*dy);
        dx /= dn;
        dy /= dn;

        while(remain < dn && len < *num2){
            x0 += dx * remain;
            pts_out[len][0] = x0;
            y0 += dy * remain;
            pts_out[len][1] = y0;
            
            len++;
            dn -= remain;
            remain = dist;
        }
        remain -= dn;
    }
    *num2 = len;
}
// 点集等距采样2  使采样后点与点的距离为`dist`
// TODO: fix bug
void resample_points2(float pts_in[][2], int num1, float pts_out[][2], int *num2, float dist) {
    if (num1 < 0) {
        *num2 = 0;
        return;
    }
    pts_out[0][0] = pts_in[0][0];
    pts_out[0][1] = pts_in[0][1];
    int len = 1;
    for (int i = 0; i < num1 - 1 && len < *num2; i++) {
        float x0 = pts_in[i][0];
        float y0 = pts_in[i][1];
        float x1 = pts_in[i + 1][0];
        float y1 = pts_in[i + 1][1];

        do {
            float x = pts_out[len - 1][0];
            float y = pts_out[len - 1][1];

            float dx0 = x0 - x;
            float dy0 = y0 - y;
            float dx1 = x1 - x;
            float dy1 = y1 - y;

            float dist0 = sqrt(dx0 * dx0 + dy0 * dy0);
            float dist1 = sqrt(dx1 * dx1 + dy1 * dy1);

            float r0 = (dist1 - dist) / (dist1 - dist0);
            float r1 = 1 - r0;

            if (r0 < 0 || r1 < 0) break;
            x0 = x0 * r0 + x1 * r1;
            y0 = y0 * r0 + y1 * r1;
            pts_out[len][0] = x0;
            pts_out[len][1] = y0;
            len++;
        } while (len < *num2);

    }
    *num2 = len;
}