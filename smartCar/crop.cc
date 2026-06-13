#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <time.h> // 需要引入这个头文件

#define CAM_W 640 // 摄像头物理输出分辨率
#define CAM_H 480
#define BEV_W 320 // 逆透视处理及寻框逻辑分辨率
#define BEV_H 240
#define NCNN_SIZE 64

// 查表矩阵的大小跟着 BEV 走，不占用额外内存！
int map_Y_idx[BEV_H][BEV_W];
int map_U_idx[BEV_H][BEV_W];
int map_V_idx[BEV_H][BEV_W];

// 逆透视矩阵
double change_un_Mat[3][3] = {
    {2.302721, 1.057898, -203.535203},
    {0.050803, 2.708197, -21.229317},
    {0.000263, 0.007076, 0.933370}
};

void InitYUVLookupTables() {
    for (int j = 0; j < BEV_H; j++) {
        for (int i = 0; i < BEV_W; i++) {
            // 1. 依然按 320x240 的逻辑算透视映射
            double denominator = change_un_Mat[2][0] * i + change_un_Mat[2][1] * j + change_un_Mat[2][2];
            int local_x_320 = (int)((change_un_Mat[0][0] * i + change_un_Mat[0][1] * j + change_un_Mat[0][2]) / denominator);
            int local_y_320 = (int)((change_un_Mat[1][0] * i + change_un_Mat[1][1] * j + change_un_Mat[1][2]) / denominator);

            // 2. 核心魔法：将 320 坐标映射到 640 高清物理原图上
            int cam_x = local_x_320 * 2;
            int cam_y = local_y_320 * 2;

            // 3. 这里的边界检查必须使用真实的物理分辨率 CAM_W 和 CAM_H
            if (cam_x >= 0 && cam_y >= 0 && cam_x < CAM_W && cam_y < CAM_H) {
                // 此时计算一维数组偏移，必须按 CAM_W (640) 来算！
                int pair_idx = cam_x / 2;
                int byte_offset = (cam_y * CAM_W / 2 + pair_idx) * 4;

                map_Y_idx[j][i] = byte_offset + ((cam_x % 2 == 0) ? 0 : 2);
                map_U_idx[j][i] = byte_offset + 1;
                map_V_idx[j][i] = byte_offset + 3;
            } else {
                map_Y_idx[j][i] = -1;
                map_U_idx[j][i] = -1;
                map_V_idx[j][i] = -1;
            }
        }
    }
    std::cout << ">>> 640x480 分辨率解耦 LUT 初始化完成！" << std::endl;
}

inline void YUV2BGR(int y, int u, int v, uint8_t& b, uint8_t& g, uint8_t& r) {
    int c = y - 16;
    int d = u - 128;
    int e = v - 128;
    r = cv::saturate_cast<uint8_t>((298 * c + 409 * e + 128) >> 8);
    g = cv::saturate_cast<uint8_t>((298 * c - 100 * d - 208 * e + 128) >> 8);
    b = cv::saturate_cast<uint8_t>((298 * c + 516 * d + 128) >> 8);
}

int main() {
    InitYUVLookupTables();

    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头！" << std::endl;
        return -1;
    }

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CAM_W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_H);
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);

    cv::Mat raw_yuyv_frame;
    cv::Mat bev_binary(BEV_H, BEV_W, CV_8UC1);

#ifdef OFFLINE_DEBUG
    cv::Mat display_original;
    cv::Mat display_bev_color(CAM_H, CAM_W, CV_8UC3);
    std::cout << ">>> 运行模式: PC 离线可视化调试" << std::endl;
#else
    std::cout << ">>> 运行模式: 嵌入式无头部署 (Headless)" << std::endl;
#endif

    while (true) {
        // ======================= 【计时点 0：总起 / 取流阶段】 =======================
        auto t_start = std::chrono::high_resolution_clock::now();
        auto t_cap_end = std::chrono::high_resolution_clock::now();
        auto wall_start = std::chrono::high_resolution_clock::now();

        struct timespec cpu_start, cpu_end;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_start);

        // ================= 取流操作 =================
        cap >> raw_yuyv_frame;
        if (raw_yuyv_frame.empty()) continue;
        // ============================================

        // 2. 获取结束的【挂钟时间】和【真实 CPU 时间】
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_end);
        auto wall_end = std::chrono::high_resolution_clock::now();

        // 3. 计算耗时
        long long wall_time_us = std::chrono::duration_cast<std::chrono::microseconds>(wall_end - wall_start).count();

        // CPU 时间差计算 (纳秒转微秒)
        long long cpu_time_us = (cpu_end.tv_sec - cpu_start.tv_sec) * 1000000LL +
                                (cpu_end.tv_nsec - cpu_start.tv_nsec) / 1000LL;

        std::cout << "[取流真实验证] 挂钟流逝: " << wall_time_us << " us | "
                << "真正吃掉的CPU算力: " << cpu_time_us << " us" << std::endl;

        const uint8_t* raw_ptr = raw_yuyv_frame.ptr<uint8_t>(0);

#ifdef OFFLINE_DEBUG
        cv::cvtColor(raw_yuyv_frame, display_original, cv::COLOR_YUV2BGR_YUYV);
#endif

        // ======================= 【计时点 1：极速 V 通道二值化】 =======================
        auto t_v_start = std::chrono::high_resolution_clock::now();

        for (int j = 0; j < BEV_H; j++) {
            for (int i = 0; i < BEV_W; i++) {
                int v_idx = map_V_idx[j][i];
                bev_binary.at<uint8_t>(j, i) = (v_idx >= 0 && raw_ptr[v_idx] > 170) ? 255 : 0;
            }
        }

        auto t_v_end = std::chrono::high_resolution_clock::now();

        // 寻轮廓 (耗时极低，通常并入几何推导统计)
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(bev_binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

#ifdef OFFLINE_DEBUG
        cv::cvtColor(bev_binary, display_bev_color, cv::COLOR_GRAY2BGR);
#endif

        // 用于累加多个目标的总耗时
        long long time_geom_total = 0;
        long long time_crop_total = 0;
        int target_count = 0;

        for (const auto& cnt : contours) {
            if (cv::contourArea(cnt) < 50) continue;

            // ======================= 【计时点 2：几何推导与探针】 =======================
            auto t_geom_start = std::chrono::high_resolution_clock::now();

            cv::RotatedRect rect = cv::minAreaRect(cnt);
            cv::Point2f pts[4];
            rect.points(pts);

#ifdef OFFLINE_DEBUG
            for (int i = 0; i < 4; i++) cv::line(display_bev_color, pts[i], pts[(i+1)%4], cv::Scalar(0, 0, 255), 2);
#endif

            float l0 = cv::norm(pts[1] - pts[0]);
            float l1 = cv::norm(pts[2] - pts[1]);
            if (std::max(l0, l1) / (std::min(l0, l1) + 1e-5) < 2.0) {
                auto t_geom_abort = std::chrono::high_resolution_clock::now();
                time_geom_total += std::chrono::duration_cast<std::chrono::microseconds>(t_geom_abort - t_geom_start).count();
                continue;
            }

            cv::Point2f v_long = (l0 > l1) ? (pts[1] - pts[0]) : (pts[2] - pts[1]);
            float L = std::max(l0, l1);
            float W = std::min(l0, l1);

            v_long = v_long / cv::norm(v_long);
            cv::Point2f v_normal(-v_long.y, v_long.x);

            float dist = W / 2.0f + L / 2.0f;
            cv::Point2f C1 = rect.center + v_normal * dist;
            cv::Point2f C2 = rect.center - v_normal * dist;

            auto evaluate_texture = [&](cv::Point2f center) {
                int cx = center.x, cy = center.y, r = 5;
                float sum = 0, sq_sum = 0; int count = 0;
                for(int y = cy - r; y <= cy + r; y++) {
                    for(int x = cx - r; x <= cx + r; x++) {
                        if(x >= 0 && x < BEV_W && y >= 0 && y < BEV_H) {
                            int y_idx = map_Y_idx[y][x];
                            if(y_idx >= 0) {
                                uint8_t val = raw_ptr[y_idx];
                                sum += val; sq_sum += val * val; count++;
                            }
                        }
                    }
                }
                if (count < 10) return 0.0f;
                float mean = sum / count;
                return (sq_sum / count) - (mean * mean);
            };

            float score1 = evaluate_texture(C1);
            float score2 = evaluate_texture(C2);
            cv::Point2f target_C = (score1 > score2) ? C1 : C2;

            auto t_geom_end = std::chrono::high_resolution_clock::now();
            time_geom_total += std::chrono::duration_cast<std::chrono::microseconds>(t_geom_end - t_geom_start).count();

            // ======================= 【计时点 3：动态高清切图】 =======================
            auto t_crop_start = std::chrono::high_resolution_clock::now();

            cv::Mat ncnn_input(NCNN_SIZE, NCNN_SIZE, CV_8UC3, cv::Scalar(128, 128, 128));

            float top_left_x = target_C.x - L / 2.0f;
            float top_left_y = target_C.y - L / 2.0f;

            for (int r = 0; r < NCNN_SIZE; r++) {
                for (int c = 0; c < NCNN_SIZE; c++) {
                    float bev_x = top_left_x + c * (L / (float)NCNN_SIZE);
                    float bev_y = top_left_y + r * (L / (float)NCNN_SIZE);

                    double denominator = change_un_Mat[2][0] * bev_x + change_un_Mat[2][1] * bev_y + change_un_Mat[2][2];
                    double cam_x_320 = (change_un_Mat[0][0] * bev_x + change_un_Mat[0][1] * bev_y + change_un_Mat[0][2]) / denominator;
                    double cam_y_320 = (change_un_Mat[1][0] * bev_x + change_un_Mat[1][1] * bev_y + change_un_Mat[1][2]) / denominator;

                    int cam_x_640 = (int)(cam_x_320 * 2.0);
                    int cam_y_640 = (int)(cam_y_320 * 2.0);

                    if (cam_x_640 >= 0 && cam_y_640 >= 0 && cam_x_640 < CAM_W && cam_y_640 < CAM_H) {
                        int pair_idx = cam_x_640 / 2;
                        int byte_offset = (cam_y_640 * CAM_W / 2 + pair_idx) * 4;

                        int y_idx = byte_offset + ((cam_x_640 % 2 == 0) ? 0 : 2);
                        int u_idx = byte_offset + 1;
                        int v_idx = byte_offset + 3;

                        uint8_t B, G, R;
                        YUV2BGR(raw_ptr[y_idx], raw_ptr[u_idx], raw_ptr[v_idx], B, G, R);

                        cv::Vec3b& pixel = ncnn_input.at<cv::Vec3b>(r, c);
                        pixel[0] = B; pixel[1] = G; pixel[2] = R;
                    }
                }
            }

            auto t_crop_end = std::chrono::high_resolution_clock::now();
            time_crop_total += std::chrono::duration_cast<std::chrono::microseconds>(t_crop_end - t_crop_start).count();

            target_count++;

#ifdef OFFLINE_DEBUG
            cv::Rect display_rect((int)top_left_x, (int)top_left_y, (int)L, (int)L);
            cv::rectangle(display_bev_color, display_rect, cv::Scalar(0, 255, 0), 2);
            cv::imshow("3. NCNN Ready 64x64", ncnn_input);
#endif
        }

        // ======================= 【耗时汇总打印】 =======================
        auto t_end = std::chrono::high_resolution_clock::now();

        auto time_cap = std::chrono::duration_cast<std::chrono::microseconds>(t_cap_end - t_start).count();
        auto time_v = std::chrono::duration_cast<std::chrono::microseconds>(t_v_end - t_v_start).count();
        auto time_total = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

        std::cout << "[帧剖析] 总计: " << time_total << " us | "
                  << "取流(Cap): " << time_cap << " us | "
                  << "V二值化: " << time_v << " us | "
                  << "寻框(" << target_count << "个): " << time_geom_total << " us | "
                  << "切图(" << target_count << "个): " << time_crop_total << " us" << std::endl;

#ifdef OFFLINE_DEBUG
        cv::imshow("1. Original Color Camera", display_original);
        cv::imshow("2. Pipeline BEV Tracker", display_bev_color);
        if (cv::waitKey(1) == 'q') break;
#endif
    }

    cap.release();
#ifdef OFFLINE_DEBUG
    cv::destroyAllWindows();
#endif
    return 0;
}

