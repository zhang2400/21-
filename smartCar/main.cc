#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <csignal>

#include <opencv2/opencv.hpp>
#include <net.h>
#include <allocator.h>

#include "ww_camera_server.h"

struct Detection {
    cv::Rect2f box;
    int label;
    float score;
};

static std::atomic<bool> g_running(true);
static ncnn::UnlockedPoolAllocator g_blob_allocator;
static ncnn::UnlockedPoolAllocator g_workspace_allocator;

static void on_signal(int)
{
    g_running = false;
}

static void install_signal_handlers()
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
}

static bool file_exists(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    return f.good();
}

static std::string first_existing_path(const std::vector<std::string>& paths)
{
    for (size_t i = 0; i < paths.size(); ++i) {
        if (file_exists(paths[i])) return paths[i];
    }
    return paths.empty() ? std::string() : paths[0];
}

static bool parse_int(const std::string& text, int* value)
{
    char* end = NULL;
    long v = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') return false;
    *value = (int)v;
    return true;
}

static float intersection_area(const Detection& a, const Detection& b)
{
    const float x1 = std::max(a.box.x, b.box.x);
    const float y1 = std::max(a.box.y, b.box.y);
    const float x2 = std::min(a.box.x + a.box.width, b.box.x + b.box.width);
    const float y2 = std::min(a.box.y + a.box.height, b.box.y + b.box.height);
    const float w = x2 - x1;
    const float h = y2 - y1;
    if (w <= 0.f || h <= 0.f) return 0.f;
    return w * h;
}

static void nms_sorted_bboxes(const std::vector<Detection>& dets, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();
    const int n = (int)dets.size();
    std::vector<float> areas(n);
    for (int i = 0; i < n; ++i) {
        areas[i] = dets[i].box.area();
    }

    for (int i = 0; i < n; ++i) {
        const Detection& a = dets[i];
        bool keep = true;
        for (size_t j = 0; j < picked.size(); ++j) {
            const Detection& b = dets[picked[j]];
            const float inter_area = intersection_area(a, b);
            const float union_area = areas[i] + areas[picked[j]] - inter_area;
            if (union_area > 0.f && inter_area / union_area > nms_threshold) {
                keep = false;
                break;
            }
        }
        if (keep) picked.push_back(i);
    }
}

static bool load_model(ncnn::Net& net, const std::string& param_path, const std::string& bin_path, int threads)
{
    net.opt.use_vulkan_compute = false;
    net.opt.num_threads = threads;
    net.opt.lightmode = true;
    net.opt.blob_allocator = &g_blob_allocator;
    net.opt.workspace_allocator = &g_workspace_allocator;
    net.opt.use_fp16_packed = false;
    net.opt.use_fp16_storage = false;
    net.opt.use_fp16_arithmetic = false;

    if (net.load_param(param_path.c_str()) != 0) {
        std::cerr << "load param failed: " << param_path << std::endl;
        return false;
    }
    if (net.load_model(bin_path.c_str()) != 0) {
        std::cerr << "load bin failed: " << bin_path << std::endl;
        return false;
    }
    return true;
}

static std::vector<Detection> infer_one_frame(
    ncnn::Net& net,
    const cv::Mat& bgr,
    int input_size,
    float score_thresh,
    float nms_thresh)
{
    const int img_w = bgr.cols;
    const int img_h = bgr.rows;

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(
        bgr.data,
        ncnn::Mat::PIXEL_BGR,
        img_w,
        img_h,
        input_size,
        input_size);

    const float norm_vals[3] = {1.f / 255.f, 1.f / 255.f, 1.f / 255.f};
    in.substract_mean_normalize(NULL, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    ex.set_light_mode(true);
    ex.set_blob_allocator(&g_blob_allocator);
    ex.set_workspace_allocator(&g_workspace_allocator);
    ex.input("in0", in);

    ncnn::Mat out;
    if (ex.extract("out0", out) != 0) {
        return std::vector<Detection>();
    }

    const int num_anchors = out.h;
    const int feat_dim = out.w;
    if (num_anchors <= 0 || feat_dim < 5) {
        return std::vector<Detection>();
    }

    const int num_classes = feat_dim - 4;
    const float scale_x = (float)img_w / (float)input_size;
    const float scale_y = (float)img_h / (float)input_size;

    std::vector<Detection> proposals;
    proposals.reserve(num_anchors / 4);

    for (int i = 0; i < num_anchors; ++i) {
        const float* row = out.row(i);
        const float x1 = row[0] * scale_x;
        const float y1 = row[1] * scale_y;
        const float x2 = row[2] * scale_x;
        const float y2 = row[3] * scale_y;

        int best_label = -1;
        float best_score = 0.f;
        for (int c = 0; c < num_classes; ++c) {
            const float score = row[4 + c];
            if (score > best_score) {
                best_score = score;
                best_label = c;
            }
        }

        if (best_score < score_thresh || best_label < 0) continue;

        Detection obj;
        obj.label = best_label;
        obj.score = best_score;
        obj.box.x = std::max(0.f, std::min(x1, (float)img_w - 1.f));
        obj.box.y = std::max(0.f, std::min(y1, (float)img_h - 1.f));
        obj.box.width = std::max(0.f, std::min(x2, (float)img_w - 1.f) - obj.box.x);
        obj.box.height = std::max(0.f, std::min(y2, (float)img_h - 1.f) - obj.box.y);
        if (obj.box.width <= 1.f || obj.box.height <= 1.f) continue;
        proposals.push_back(obj);
    }

    std::sort(proposals.begin(), proposals.end(), [](const Detection& a, const Detection& b) {
        return a.score > b.score;
    });

    std::vector<Detection> results;
    for (int c = 0; c < num_classes; ++c) {
        std::vector<Detection> class_dets;
        for (size_t i = 0; i < proposals.size(); ++i) {
            if (proposals[i].label == c) class_dets.push_back(proposals[i]);
        }
        if (class_dets.empty()) continue;

        std::vector<int> picked;
        nms_sorted_bboxes(class_dets, picked, nms_thresh);
        for (size_t i = 0; i < picked.size(); ++i) {
            results.push_back(class_dets[picked[i]]);
        }
    }

    return results;
}

static void draw_results(cv::Mat& frame, const std::vector<Detection>& dets, double infer_ms, double fps)
{
    for (size_t i = 0; i < dets.size(); ++i) {
        const Detection& d = dets[i];
        cv::rectangle(frame, d.box, cv::Scalar(0, 255, 0), 2);

        char label[64];
        std::snprintf(label, sizeof(label), "cls:%d %.2f", d.label, d.score);
        int base_line = 0;
        cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.45, 1, &base_line);
        int x = std::max(0, (int)d.box.x);
        int y = std::max(text_size.height + 3, (int)d.box.y);
        cv::rectangle(frame,
                      cv::Rect(x, y - text_size.height - 3, text_size.width + 4, text_size.height + base_line + 4),
                      cv::Scalar(0, 255, 0),
                      -1);
        cv::putText(frame, label, cv::Point(x + 2, y - 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 0), 1);
    }

    char speed_text[96];
    std::snprintf(speed_text, sizeof(speed_text), "infer %.1f ms  %.1f FPS  dets %zu", infer_ms, fps, dets.size());
    cv::putText(frame, speed_text, cv::Point(5, 18),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
}

static std::string format_detections(const std::vector<Detection>& dets)
{
    std::ostringstream oss;
    for (size_t i = 0; i < dets.size(); ++i) {
        const Detection& d = dets[i];
        oss << " [cls=" << d.label
            << " score=" << d.score
            << " x=" << (int)d.box.x
            << " y=" << (int)d.box.y
            << " w=" << (int)d.box.width
            << " h=" << (int)d.box.height
            << "]";
    }
    return oss.str();
}

static void print_usage(const char* prog)
{
    std::cout
        << "Usage / 使用方法:\n"
        << "  " << prog << " [camera_index|video_path] [param_path] [bin_path] [score_thresh] [nms_thresh] [input_size] [server_port]\n\n"
        << "  " << prog << " ... [server_port] [threads] [camera_width] [camera_height] [print_interval]\n\n"
        << "Parameters / 参数说明:\n"
        << "  camera_index|video_path  摄像头编号或视频文件路径，默认 0 表示打开 /dev/video0\n"
        << "  param_path               ncnn param 模型结构文件路径\n"
        << "  bin_path                 ncnn bin 模型权重文件路径\n"
        << "  score_thresh             置信度阈值，低于该值的检测框会被丢弃，默认 0.35\n"
        << "  nms_thresh               NMS 去重阈值，用于去掉重叠检测框，默认 0.45\n"
        << "  input_size               模型输入尺寸，越小越快但精度可能下降，默认 224\n"
        << "  server_port              图传网页端口，默认 8080，设置为 0 可关闭图传\n"
        << "  threads                  ncnn 推理线程数，默认 4\n"
        << "  camera_width             摄像头采集宽度，默认 160\n"
        << "  camera_height            摄像头采集高度，默认 120\n"
        << "  print_interval           终端打印间隔，默认每 10 帧打印一次\n\n"
        << "Defaults / 默认值:\n"
        << "  source                   0\n"
        << "  param_path               自动查找 ./model.ncnn.param, ./smartCar/model.ncnn.param, ../smartCar/model.ncnn.param\n"
        << "  bin_path                 自动查找 ./model.ncnn.bin, ./smartCar/model.ncnn.bin, ../smartCar/model.ncnn.bin\n"
        << "  score_thresh             0.35\n"
        << "  nms_thresh               0.45\n"
        << "  input_size               224\n"
        << "  server_port              8080\n"
        << "  threads                  4\n"
        << "  camera                   160x120\n"
        << "  print_interval           10\n";
}

int main(int argc, char** argv)
{
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        print_usage(argv[0]);
        return 0;
    }

    install_signal_handlers();

    const std::string source = argc > 1 ? argv[1] : "0";
    const std::string param_path = argc > 2 ? argv[2] : first_existing_path(std::vector<std::string>{
        "./model.ncnn.param",
        "./smartCar/model.ncnn.param",
        "../smartCar/model.ncnn.param"});
    const std::string bin_path = argc > 3 ? argv[3] : first_existing_path(std::vector<std::string>{
        "./model.ncnn.bin",
        "./smartCar/model.ncnn.bin",
        "../smartCar/model.ncnn.bin"});
    const float score_thresh = argc > 4 ? std::strtof(argv[4], NULL) : 0.35f;
    const float nms_thresh = argc > 5 ? std::strtof(argv[5], NULL) : 0.45f;
    const int input_size = argc > 6 ? std::atoi(argv[6]) : 224;
    const int server_port = argc > 7 ? std::atoi(argv[7]) : 8080;
    const int threads = argc > 8 ? std::atoi(argv[8]) : 4;
    const int camera_width = argc > 9 ? std::atoi(argv[9]) : 160;
    const int camera_height = argc > 10 ? std::atoi(argv[10]) : 120;
    const int print_interval = argc > 11 ? std::atoi(argv[11]) : 10;

    if (input_size <= 0) {
        std::cerr << "invalid input_size: " << input_size << std::endl;
        return 1;
    }

    ncnn::Net net;
    if (!load_model(net, param_path, bin_path, threads)) {
        return 2;
    }

    cv::VideoCapture cap;
    int camera_index = 0;
    if (parse_int(source, &camera_index)) {
        cap.open(camera_index);
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH, camera_width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, camera_height);
        cap.set(cv::CAP_PROP_FPS, 60);
    } else {
        cap.open(source);
    }

    if (!cap.isOpened()) {
        std::cerr << "open video source failed: " << source << std::endl;
        return 3;
    }

    CameraStreamServer server;
    if (server_port > 0) {
        if (server.start_server(server_port) < 0) {
            std::cerr << "stream server start failed, continue without stream server" << std::endl;
        }
        install_signal_handlers();
    }

    std::cout << "ncnn camera detect started\n"
              << "source=" << source
              << " param=" << param_path
              << " bin=" << bin_path
              << " score_thresh=" << score_thresh
              << " nms_thresh=" << nms_thresh
              << " input_size=" << input_size
              << " threads=" << threads
              << " camera=" << camera_width << "x" << camera_height
              << " print_interval=" << print_interval
              << std::endl;

    int frame_id = 0;
    double ema_fps = 0.0;
    cv::Mat frame;

    while (g_running && cap.read(frame)) {
        if (frame.empty()) continue;

        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        std::vector<Detection> dets = infer_one_frame(net, frame, input_size, score_thresh, nms_thresh);
        const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        const double infer_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        const double fps = infer_ms > 0.0 ? 1000.0 / infer_ms : 0.0;
        ema_fps = frame_id == 0 ? fps : ema_fps * 0.9 + fps * 0.1;

        draw_results(frame, dets, infer_ms, ema_fps);
        if (server_port > 0 && server.is_running()) {
            server.update_frame(frame);
        }

        if (print_interval <= 1 || frame_id % print_interval == 0 || !dets.empty()) {
            std::cout << "frame=" << frame_id
                      << " detections=" << dets.size()
                      << " infer_ms=" << infer_ms
                      << " fps=" << fps
                      << " avg_fps=" << ema_fps
                      << format_detections(dets)
                      << std::endl;
        }

        ++frame_id;
    }

    server.stop_server();
    std::cout << "detect stopped, total_frames=" << frame_id << std::endl;
    return 0;
}
