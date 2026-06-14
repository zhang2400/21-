#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <atomic>   // 解决 incomplete type 报错的关键
#include <string>
#include <net.h>
#include <sys/stat.h> // 用于检查/创建文件夹
#include <thread>

#define CAM_W 640 // 摄像头物理输出分辨率
#define CAM_H 480
#define BEV_W 320 // 逆透视处理及寻框逻辑分辨率
#define BEV_H 240
#define NCNN_SIZE 64
std::atomic<bool> g_save_requested(false);
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

void TerminalListenerThread() {
    std::string cmd;
    while (true) {
        std::cin >> cmd;
        if (cmd == "s" || cmd == "save") {
            g_save_requested = true;
            std::cout << "\n[互动提示] >>> 收到保存指令！将保存下一个检测到的目标图片..." << std::endl;
        }
    }
}

// ======================= NCNN 辅助函数 =======================
static int parse_int_arg(int argc, char** argv, int index, int default_value) {
    if (argc <= index) return default_value;
    return std::atoi(argv[index]);
}

static const char* parse_str_arg(int argc, char** argv, int index, const char* default_value) {
    if (argc <= index) return default_value;
    return argv[index];
}

static bool load_cls_model(ncnn::Net& net, const char* param_path, const char* bin_path, int threads) {
    net.opt.num_threads = threads;
    net.opt.use_vulkan_compute = false;
    net.opt.use_fp16_packed = false;
    net.opt.use_fp16_storage = false;
    net.opt.use_fp16_arithmetic = false;
    net.opt.use_packing_layout = false;

    if (net.load_param(param_path) != 0) {
        std::cerr << "Failed to load param: " << param_path << std::endl;
        return false;
    }
    if (net.load_model(bin_path) != 0) {
        std::cerr << "Failed to load bin: " << bin_path << std::endl;
        return false;
    }
    std::cout << ">>> NCNN 模型加载成功: " << param_path << " / " << bin_path << std::endl;
    return true;
}

static bool classify_image(ncnn::Net& net, const cv::Mat& bgr, int input_size, std::vector<float>& scores) {
    ncnn::Mat input = ncnn::Mat::from_pixels_resize(
        bgr.data,
        ncnn::Mat::PIXEL_BGR2RGB,
        bgr.cols,
        bgr.rows,
        input_size,
        input_size);

    const float mean_vals[3] = {0.f, 0.f, 0.f};
    const float norm_vals[3] = {1.f / 255.f, 1.f / 255.f, 1.f / 255.f};
    input.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    ex.set_light_mode(true);

    if (ex.input("input", input) != 0) {
        std::cerr << "Failed to set input node: input" << std::endl;
        return false;
    }

    ncnn::Mat output;
    if (ex.extract("output", output) != 0) {
        std::cerr << "Failed to extract output node: output" << std::endl;
        return false;
    }

    scores.resize(output.w * output.h * output.c);
    for (int i = 0; i < (int)scores.size(); i++) {
        scores[i] = output[i];
    }
    return true;
}

// ======================= 预处理辅助函数 =======================
void InitYUVLookupTables() {
    for (int j = 0; j < BEV_H; j++) {
        for (int i = 0; i < BEV_W; i++) {
            double denominator = change_un_Mat[2][0] * i + change_un_Mat[2][1] * j + change_un_Mat[2][2];
            int local_x_320 = (int)((change_un_Mat[0][0] * i + change_un_Mat[0][1] * j + change_un_Mat[0][2]) / denominator);
            int local_y_320 = (int)((change_un_Mat[1][0] * i + change_un_Mat[1][1] * j + change_un_Mat[1][2]) / denominator);

            int cam_x = local_x_320 * 2;
            int cam_y = local_y_320 * 2;

            if (cam_x >= 0 && cam_y >= 0 && cam_x < CAM_W && cam_y < CAM_H) {
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

// ======================= 主函数 =======================
int main(int argc, char** argv) {
    // 1. 初始化 NCNN 模型
    // 使用说明: ./程序名 [param路径] [bin路径] [线程数]
    const char* param_path = parse_str_arg(argc, argv, 1, "./cls.param");
    const char* bin_path   = parse_str_arg(argc, argv, 2, "./cls.bin");
    const int threads      = std::max(1, parse_int_arg(argc, argv, 3, 1));
    std::thread t_listener(TerminalListenerThread);
    t_listener.detach();
    std::cout << ">>> 终端交互已启动！在无头模式下，随时在终端输入 's' 并回车即可抓取当前目标图片。" << std::endl;
    ncnn::Net net;
    if (!load_cls_model(net, param_path, bin_path, threads)) {
        return -1;
    }

    // 2. 初始化 LUT
    InitYUVLookupTables();

    // 3. 启动摄像头
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
        cap >> raw_yuyv_frame;
        if (raw_yuyv_frame.empty()) continue;

        const uint8_t* raw_ptr = raw_yuyv_frame.ptr<uint8_t>(0);

#ifdef OFFLINE_DEBUG
        cv::cvtColor(raw_yuyv_frame, display_original, cv::COLOR_YUV2BGR_YUYV);
#endif

        // 极速 V 通道二值化
        for (int j = 0; j < BEV_H; j++) {
            for (int i = 0; i < BEV_W; i++) {
                int v_idx = map_V_idx[j][i];
                bev_binary.at<uint8_t>(j, i) = (v_idx >= 0 && raw_ptr[v_idx] > 170) ? 255 : 0;
            }
        }

        // 寻轮廓与纯几何推导
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(bev_binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

#ifdef OFFLINE_DEBUG
        cv::cvtColor(bev_binary, display_bev_color, cv::COLOR_GRAY2BGR);
#endif

        for (const auto& cnt : contours) {
            if (cv::contourArea(cnt) < 50) continue;

            cv::RotatedRect rect = cv::minAreaRect(cnt);
            cv::Point2f pts[4];
            rect.points(pts);

#ifdef OFFLINE_DEBUG
            for (int i = 0; i < 4; i++) cv::line(display_bev_color, pts[i], pts[(i+1)%4], cv::Scalar(0, 0, 255), 2);
#endif

            float l0 = cv::norm(pts[1] - pts[0]);
            float l1 = cv::norm(pts[2] - pts[1]);
            if (std::max(l0, l1) / (std::min(l0, l1) + 1e-5) < 2.0) continue;

            cv::Point2f v_long = (l0 > l1) ? (pts[1] - pts[0]) : (pts[2] - pts[1]);
            float L = std::max(l0, l1);
            float W = std::min(l0, l1);

            v_long = v_long / cv::norm(v_long);
            cv::Point2f v_normal(-v_long.y, v_long.x);

            // Y 通道探针懒加载测向
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

            // 懒加载生成 NCNN 专属全彩输入
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

            // ================= 接入 NCNN 推理 =================
            std::vector<float> scores;
            if (classify_image(net, ncnn_input, NCNN_SIZE, scores) && !scores.empty()) {
                const int top_id = (int)(std::max_element(scores.begin(), scores.end()) - scores.begin());
                const float top_score = scores[top_id];

                // 打印推理结果
                std::cout << "[Target C: (" << target_C.x << ", " << target_C.y << ")] "
                          << "Top1 Class: " << top_id
                          << ", Score: " << std::fixed << std::setprecision(4) << top_score << std::endl;

                // ====== 新增：终端触发交互式保存 ======
                if (g_save_requested) {
                    g_save_requested = false; // 消费掉这次保存请求，重置状态

                    // 获取毫秒级时间戳避免文件名冲突
                    auto now = std::chrono::system_clock::now();
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                    // 巧妙命名法：文件名包含 [时间戳]_[模型预测ID]_[置信度].png
                    // 这样识别不准时，你通过文件名一眼就能看出模型把这个图错分成了谁，极方便后续归类
                    std::stringstream ss;
                    ss << "saved_dataset/img_" << ms << "_pred" << top_id << "_conf" << std::fixed << std::setprecision(2) << top_score << ".png";
                    std::string filename = ss.str();

                    // 执行保存
                    if (cv::imwrite(filename, ncnn_input)) {
                        std::cout << "========================================" << std::endl;
                        std::cout << "[成功保存] 图片已导出至: " << filename << std::endl;
                        std::cout << "========================================" << std::endl;
                    } else {
                        std::cerr << "[错误] 保存图片失败，请检查 saved_dataset 文件夹权限！" << std::endl;
                    }
                }
                // =======================================
#ifdef OFFLINE_DEBUG
                // [调试模式独有]: 画绿框并显示分类文字及 64x64 结果图
                cv::Rect display_rect(top_left_x, top_left_y, L, L);
                cv::rectangle(display_bev_color, display_rect, cv::Scalar(0, 255, 0), 2);

                // 在目标框上方绘制类别和置信度
                std::string label = "ID: " + std::to_string(top_id) + " (" + std::to_string(top_score).substr(0,4) + ")";
                cv::putText(display_bev_color, label, cv::Point(top_left_x, top_left_y - 5),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

                cv::imshow("3. NCNN Ready 64x64", ncnn_input);
#endif
            }
        }

#ifdef OFFLINE_DEBUG
        // [调试模式独有]: 统一渲染所有窗口并处理键盘退出
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

