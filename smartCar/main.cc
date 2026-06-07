#include "headfile.h"
#include "parameter.h"
#include "Img_process.h"
#include "PID.h"
#include "garage.h"
#include "utils.h"
#include "cross.h"
#include "motor.h"
#include "circle.h"
#include "rot.h"
#include "encoder.h"
#include "turn.h"
using namespace cv;


Motor motor;
LCD lcd;

Mat img;
Mat bird_img; 
Mat img_raw;
Mat img_mb;
Mat img_bin;
Mat transform_matrix;  // 透视变换矩阵
bool transform_initialized = false;

int track_with = 45;// 巡线宽度（像素）
int zebra_width = 0;// 斑马线宽度（像素）
float angle;// 当前转角

extern int direction_l;
extern int direction_r;



// 当前巡线模式
enum track_type_e track_type = TRACK_LEFT;
void process_img(void);


void imu_get_thread(void *arg);

int main() 
{  
    motor.motor_init();
    
    // motor.set_motor1(0, 3000);
    // motor.set_motor2(0, 3000);
    
    // TimerThread thread_1(imu_get_thread, NULL, 10);
    CameraStreamServer server;
    if(server.start_server(8080) < 0) {
        return -1;
    }
    //wait for start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    lcd.lcd_init();
    motor_PID_init(&motor_l, 0, 1000, 250, 100, 5000, 500, 600, 1, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX);
    motor_PID_init(&motor_r, 0, 1000, 250, 100, 5000, 500, 600, 1, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX);
    // 原来的 kp=7021 太大，改成合适的值
    pid_init(&motor_pid_l, 6.0f, 0.8f, 0.12f, 0.8f, 500.0f, 500.0f, 500.0f);  // 限幅也调小
    pid_init(&motor_pid_r, 6.0f, 0.8f, 0.12f, 0.8f, 500.0f, 500.0f, 500.0f);
    pid_init(&target_speed_pid, 5.0, 0, 30.0, 0.6, 5, 5, 5);
    pid_init(&posloop_pid, 200., 0, 0., 0.7, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX, MOTOR_PWM_DUTY_MAX);
    pid_init(&servo_pid, 1.2f, 0.0f, 0.1f, 0.8f, 14.5f, 0.0f, 14.5f);
    VideoCapture cap(0);
    cap.set(CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, 160);
    cap.set(CAP_PROP_FRAME_HEIGHT, 120);
    cap.set(CAP_PROP_FPS,60);
    
    // // ========== 添加透视变换部分 ==========
    // // 初始化透视变换矩阵（只需要初始化一次）
    // if (!transform_initialized) {
    //     transform_matrix = getPerspectiveTransform(src_vertices, dst_vertices);
    //     transform_initialized = true;
        
    //     // 可选：打印变换矩阵，用于调试
    //     // std::cout << "Transform matrix: " << transform_matrix << std::endl;
    // }
    
 
    // thread_1.start();
    while(server.is_running()) 
    // while(1)
    {
        cap >> img;
        if(img.empty()) {
            break;
        }    
        
        
    //    process_img();
    //    find_corners();
        aim_distance = 0.58;
        // for(int i = 0; i < left_line.len; i++) {
        //     circle(bird_img, Point(left_rp[i][0], left_rp[i][1]), 1, Scalar(255,0,0), -1);
        // }
        // // for(int i = 0; i < 60; i++) {
        // //     circle(bird_img, Point(left_line.center_line[i][0], left_line.center_line[i][1]), 1, Scalar(0,0,255), -1);
        // // }
        // for(int i = 0; i < right_line.len; i++) {
        //     circle(bird_img, Point(right_b[i][0], right_b[i][1]), 1, Scalar(0,255,0), -1);
        // }
        // 临时测试：画一条水平线
        // for(int i = 0; i < 50; i++) {
        //     circle(img, Point(50 + i, 60), 2, Scalar(0,255,0), -1);
        // }

        // detect_zebra_cross_horizontal(70, 80, &DEFAULT_PARAMS, NULL, &zebra_width, img_bin);
        server.update_frame(img);

        // 单侧线少，切换巡线方向  切外向圆
        if (rpts0s_num < rpts1s_num / 2 && rpts0s_num < 60) {
            track_type = TRACK_RIGHT;
        } else if (rpts1s_num < rpts0s_num / 2 && rpts1s_num < 60) {
            track_type = TRACK_LEFT;
        } else if (rpts0s_num < 20 && rpts1s_num > rpts0s_num) {
            track_type = TRACK_RIGHT;
        } else if (rpts1s_num < 20 && rpts0s_num > rpts1s_num) {
            track_type = TRACK_LEFT;
        }
        // 车库斑马线检查(斑马线优先级高，最先检查)
        // check_garage();
        // 总钻风检查Apriltag(找赛道上的黑斑)
        //先空着

        // 分别检查十字 三叉 和圆环, 十字优先级最高
        // if (garage_type == GARAGE_NONE &&
        //     apriltag_type != APRILTAG_FOUND && apriltag_type != APRILTAG_LEAVE)
        //     check_cross();
        // if (garage_type == GARAGE_NONE && cross_type == CROSS_NONE && circle_type == CIRCLE_NONE &&
        //     apriltag_type != APRILTAG_FOUND && apriltag_type != APRILTAG_LEAVE)
        //     check_yroad();
        // if (garage_type == GARAGE_NONE && cross_type == CROSS_NONE && yroad_type == YROAD_NONE &&
        //     apriltag_type != APRILTAG_FOUND && apriltag_type != APRILTAG_LEAVE)
        //     check_circle();
        // if (cross_type != CROSS_NONE) {
        //     circle_type = CIRCLE_NONE;
        //     yroad_type = YROAD_NONE;
        // }
        if (garage_type == GARAGE_NONE)
            check_cross();
        if (garage_type == GARAGE_NONE && cross_type == CROSS_NONE ) 
            check_circle();
        if (cross_type != CROSS_NONE) {
            circle_type = CIRCLE_NONE;
        }
       
        
      

        // //车库 ,十字清Aprltag标志
        // if (garage_type != GARAGE_NONE || cross_type != CROSS_NONE) apriltag_type = APRILTAG_NONE;

        // //根据检查结果执行模式
        // if (yroad_type != YROAD_NONE) run_yroad();
        if (cross_type != CROSS_NONE) run_cross();
        if (circle_type != CIRCLE_NONE) run_circle();
        // if (garage_type != GARAGE_NONE) run_garage();

        // 中线跟踪
        if (cross_type != CROSS_IN) {
            if (track_type == TRACK_LEFT) {
                memcpy(mid_line.center_line, rptsc0, sizeof(mid_line.center_line));
                // mid_line = left_line;
                mid_line.len = rptsc0_num;
            } else {
                memcpy(mid_line.center_line, rptsc1, sizeof(mid_line.center_line));
                //mid_line = right_line;
                mid_line.len = rptsc1_num;
            }
        } 
        else {
            //十字根据远线控制
            if (track_type == TRACK_LEFT) {
               track_leftline(far_rpts0s + far_Lpt0_rpts0s_id, far_rpts0s_num - far_Lpt0_rpts0s_id, mid_line.center_line,
                               (int) round(angle_dist / sample_dist), pixel_per_meter * ROAD_WIDTH / 2);
               mid_line.len = far_rpts0s_num - far_Lpt0_rpts0s_id;
            } else {
                track_rightline(far_rpts1s + far_Lpt1_rpts1s_id, far_rpts1s_num - far_Lpt1_rpts1s_id, mid_line.center_line,
                                (int) round(angle_dist / sample_dist), pixel_per_meter * ROAD_WIDTH / 2);
                mid_line.len = far_rpts1s_num - far_Lpt1_rpts1s_id;
            }
        }

        for(int i = 0; i < left_line.len; i++) {
            circle(img, Point(left_line.line[i][0], left_line.line[i][1]), 1, Scalar(255,0,0), -1);
        }
        for(int i = 0; i < right_line.len; i++) {
            circle(img, Point(right_line.line[i][0], right_line.line[i][1]), 1, Scalar(0,0,255), -1);
        }
        for(int i = 0; i < mid_line.len; i++) {
            int pt[2];
            if(!map_inv(mid_line.center_line[i], pt)) continue;
            circle(img, Point(pt[0], pt[1]), 2, Scalar(0,255,0), -1);
        }
        // for(int i = 0; i < rptsc1_num; i++) {
        //     int pt[2];
        //     if(!map_inv(rptsc1[i], pt)) continue;
        //     circle(img, Point(pt[0], pt[1]), 2, Scalar(0,255,255), -1);
        // }
        for (int i = 0; i < rpts0an_num; i++) {
            if (left_line.angle_max[i] != 0) {  // 只显示非零的极大值点
            int pt[2];
            if(!map_inv(rpts0s[i], pt)) continue;
            circle(img, Point(pt[0],pt[1]), 3, Scalar(0, 255, 255), -1);
            // lcd.showDouble(0, 90, (double)left_line.angle[i], 5,2);
            // lcd.showDouble(30, 90, left_line.angle[clip(i+10, 0, rpts0an_num)], 5, 2);
            // lcd.showDouble(60, 90, left_line.angle[clip(i-10, 0, rpts0an_num)], 5, 2);
            // lcd.showInt(90, 90, left_line.angle[i], 3);
            }
            
        } 
        lcd.showInt(0, 90, rpts0s_num, 3);
        lcd.showInt(40, 90, rpts1s_num, 3);
        lcd.showInt(0, 100, Lpt0_found, 1); 
        lcd.showInt(20, 100, Lpt1_found, 1);
        lcd.showInt(40, 100, is_straight0, 1);
        lcd.showInt(60, 100, is_straight1, 1);
        // lcd.showString(0, 110, cross_type == CROSS_IN ? "CROSS" : "NORMAL");
        lcd.showInt(0, 110, direction_l, 2);
        lcd.showInt(60, 110, direction_r, 2);
        // lcd.showString(0, 120, circle_type != CIRCLE_NONE ? "CIRCLE" : "NO CIRCLE");
        lcd.showInt(100, 120, circle_type, 2);
        lcd.showInt(60,120, motor_r.duty,4);
        lcd.showInt(0,120, motor_l.duty,4);
        lcd.showInt(60, 130, motor_r.encoder_speed,4);
        lcd.showInt(0, 130, motor_l.encoder_speed,4);
        lcd.showDouble(100, 130, angle, 4, 2);
        lcd.showInt(60, 140, motor_r.target_speed,4);
        lcd.showInt(0, 140, motor_l.target_speed,4);
        lcd.showDouble(100, 140, adc_angle, 4, 2);
        // circle(bird_img, Point(left_line.line[Lpt0_rpts0s_id][0], left_line.line[Lpt0_rpts0s_id][1]), 3, Scalar(255,255,0), -1);
        lcd.showCVImage(0, 0, img, 128, 80);
        // lcd.showInt(0, 90, left_line.len, 3);
        // lcd.showInt(40, 90, right_line.len, 3);
        // lcd.showInt(80, 90, track_type, 1);
        // // 车轮对应点(纯跟踪起始点)
        // float cx = 80;// 车轮对应点x坐标（像素），需要根据实际情况调整
        // float cy = 170;// 车轮对应点y坐标（像素），需要根据实际情况调整
        float cx = mapx[86][86];
        float cy = mapy[86][86];

        // // 找最近点(起始点中线归一化)
        float min_dist = 1e10;
        int begin_id = -1;
        for (int i = 0; i < 40; i++) {
            float dx = mid_line.center_line[i][0] - cx;
            float dy = mid_line.center_line[i][1] - cy;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < min_dist) {
                min_dist = dist;
                begin_id = i;
            }
        }
        // 特殊模式下，不找最近点(由于边线会绕一圈回来，导致最近点为边线最后一个点，从而中线无法正常生成)
        if (cross_type == CROSS_IN) begin_id = 0;

        // 中线有点，同时最近点不是最后几个点
        if (begin_id >= 0 && mid_line.len- begin_id >= 3) {
            // 归一化中线
            mid_line.center_line[begin_id][0] = cx;
            mid_line.center_line[begin_id][1] = cy;
            mid_uinum = sizeof(mid_ui) / sizeof(mid_ui[0]);
            resample_points(mid_line.center_line + begin_id, mid_line.len - begin_id, mid_ui, &mid_uinum, sample_dist * pixel_per_meter);

            // 远预锚点位置
            int aim_idx = clip(round(aim_distance / sample_dist), 0, mid_uinum - 1);
            // 近预锚点位置
            int aim_idx_near = clip(round(0.25 / sample_dist), 0, mid_uinum - 1);

            // 计算远锚点偏差值
            float dx = mid_ui[aim_idx][0] - cx;
            float dy = cy - mid_ui[aim_idx][1] + 0.2 * pixel_per_meter;
            float dn = sqrt(dx * dx + dy * dy);
            float error = -atan2f(dx, dy) * 180 / PI;
            assert(!isnan(error));

            // 若考虑近点远点,可近似构造Stanley算法,避免撞路肩
            // 计算近锚点偏差值
            float dx_near = mid_ui[aim_idx_near][0] - cx;
            float dy_near = cy - mid_ui[aim_idx_near][1] + 0.2 * pixel_per_meter;
            float dn_near = sqrt(dx_near * dx_near + dy_near * dy_near);
            float error_near = -atan2f(dx_near, dy_near) * 180 / PI;
            assert(!isnan(error_near));

            // // 远近锚点综合考虑
            // //angle = pid_solve(&servo_pid, error * far_rate + error_near * (1 - far_rate));
            // // 根据偏差进行PD计算
            // //float angle = pid_solve(&servo_pid, error);

            // 纯跟踪算法(只考虑远点)
            float pure_angle = -atanf(pixel_per_meter * 2 * 0.2 * dx / dn / dn) / PI * 180 / SMOTOR_RATE;
            angle = pid_solve(&servo_pid, pure_angle);
            // angle = MINMAX(angle, -14.5, 14.5);

            // //非上坡电感控制,PD计算之后的值用于寻迹舵机的控制
            // if (enable_adc) {
            //     // 当前上坡模式，不控制舵机，同时清除所有标志
            //     yroad_type = YROAD_NONE;
            //     circle_type = CIRCLE_NONE;
            //     cross_type = CROSS_NONE;
            //     garage_type = GARAGE_NONE;
            //     apriltag_type = APRILTAG_NONE;
            // } else if (adc_cross && cross_type == CROSS_IN) {
            //     // 当前是十字模式，同时启用了电感过十字，则不控制舵机
            // } else {
            //     smotor1_control(servo_duty(SMOTOR1_CENTER + angle));
            // }

        } else {
            // 中线点过少(出现问题)，则不控制舵机
            mid_uinum = 0;
        }
    }
    cap.release();  
    return 0;
}




void process_img(void) {

    // 原图找左右边线    
    
    flip(img, img, -1);
    // warpPerspective(img, bird_img, transform_matrix, Size(160, 120), INTER_LINEAR);
    nitoushi_fast(img, bird_img);
    // flip(bird_img, bird_img, -1);
    cvtColor(img, img_raw, COLOR_RGB2GRAY);
    medianBlur(img_raw, img_mb, 3);
    threshold(img_mb, img_bin, 0, 255, THRESH_OTSU);

    int x = 86;
    int y = 96;
    left_line.len = EDGELINE_MAX;
    right_line.len = EDGELINE_MAX;
        
    find_rightbase(img_bin, &x, &y);
    findline_righthand_adaptive(img_bin, x, y, right_line.line, &right_line.len);
        
    int x1 = 86;
    int y1 = 96;
    find_leftbase(img_bin, &x1, &y1);
    findline_lefthand_adaptive(img_bin, x1, y1, left_line.line, &left_line.len);

    // 去畸变+透视变换
    for (int i = 0; i < left_line.len; i++) {
        rpts0[i][0] = mapx[left_line.line[i][1]][left_line.line[i][0]];
        rpts0[i][1] = mapy[left_line.line[i][1]][left_line.line[i][0]];
    }
    rpts0_num = left_line.len;
    for (int i = 0; i < right_line.len; i++) {
        rpts1[i][0] = mapx[right_line.line[i][1]][right_line.line[i][0]];
        rpts1[i][1] = mapy[right_line.line[i][1]][right_line.line[i][0]];
    }
    rpts1_num = right_line.len;

    // 边线滤波
    blur_points(rpts0, rpts0_num, rpts0b, 7);
    blur_points(rpts1, rpts1_num, rpts1b, 7);
    rpts0b_num = rpts0_num;
    rpts1b_num = rpts1_num;
    // 边线等距采样
    rpts0s_num = sizeof(rpts0s) / sizeof(rpts0s[0]);
    rpts1s_num = sizeof(rpts1s) / sizeof(rpts1s[0]);
    resample_points(rpts0b, rpts0b_num, rpts0s, &rpts0s_num, sample_dist * pixel_per_meter);
    resample_points(rpts1b, rpts1b_num, rpts1s, &rpts1s_num, sample_dist * pixel_per_meter);

    // 边线局部角度变化率
    local_angle_points(rpts0s, rpts0s_num, left_line.angle, 10);
    rpts0a_num = rpts0s_num;    
    local_angle_points(rpts1s, rpts1s_num, right_line.angle, 10);
    rpts1a_num = rpts1s_num;
    // 角度变化率非极大抑制
    nms_angle(left_line.angle, rpts0a_num, left_line.angle_max, 23);
    rpts0an_num = rpts0a_num;
    nms_angle(right_line.angle, rpts1a_num, right_line.angle_max, 23);
    rpts1an_num = rpts1a_num;
    // 左右中线跟踪
    track_leftline(rpts0s, rpts0s_num, rptsc0, 10, pixel_per_meter * ROAD_WIDTH / 2);
    rptsc0_num = rpts0s_num;
    track_rightline(rpts1s, rpts1s_num, rptsc1, 10, pixel_per_meter * ROAD_WIDTH / 2);
    rptsc1_num = rpts1s_num;
   
}

void imu_get_thread(void *arg) {
   encoder_get();
   elec_calculate();
   motor_control();

   
}