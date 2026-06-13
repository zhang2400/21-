#include "net.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    // 1. 初始化网络
    ncnn::Net yolo_cls;
    
    // 针对 LA264 单核无 SIMD 的极限配置
    yolo_cls.opt.num_threads = 1;         // 目标硬件只有 1 个 CPU
    yolo_cls.opt.use_vulkan_compute = false; 
    
    // 在无 SIMD 的标量 CPU 上，强制关闭 packing 和 fp16 存储
    // 因为纯 C 代码做内存排布转换的开销可能大于带来的收益
    yolo_cls.opt.use_fp16_packed = false; 
    yolo_cls.opt.use_fp16_storage = false;
    yolo_cls.opt.use_fp16_arithmetic = false;
    yolo_cls.opt.use_packing_layout = false;

    // 加载模型
    if (yolo_cls.load_param("model.ncnn.param") != 0 || 
        yolo_cls.load_model("model.ncnn.bin") != 0) {
        std::cerr << "Failed to load model!" << std::endl;
        return -1;
    }

    // 2. 随便生成一张 64x64 的测试图片
    const int target_size = 64;
    cv::Mat dummy_img(target_size, target_size, CV_8UC3);
    // 填入随机 RGB 值模拟真实数据分布
    cv::randu(dummy_img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));

    // 预处理 (从 BGR 转换并归一化)
    ncnn::Mat in = ncnn::Mat::from_pixels(dummy_img.data, ncnn::Mat::PIXEL_BGR2RGB, dummy_img.cols, dummy_img.rows);
    const float mean_vals[3] = {0.f, 0.f, 0.f};
    const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    // 请用 Netron 打开 .param 文件确认输入输出节点名
    // YOLOv8-cls 导出通常输入叫 "images"，输出可能是 "output" 或类似的名称
    const char* input_node = "in0"; 
    const char* output_node = "out0"; 

    // 3. 预热 (Warm-up)
    // 嵌入式设备刚唤醒时频率可能不稳，且 Cache 未命中，必须预热
    std::cout << "Warming up (10 iterations)..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        ncnn::Extractor ex = yolo_cls.create_extractor();
        ex.input(input_node, in);
        ncnn::Mat out;
        ex.extract(output_node, out);
    }

    // 4. 正式测速
    int loop_count = 100; // 跑 100 帧求平均
    std::cout << "Benchmarking (" << loop_count << " iterations)..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < loop_count; ++i) {
        ncnn::Extractor ex = yolo_cls.create_extractor();
        ex.input(input_node, in);
        ncnn::Mat out;
        ex.extract(output_node, out);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 计算统计结果
    float total_time = (float)duration.count();
    float avg_time = total_time / loop_count;
    
    std::cout << "Total Time: " << total_time << " ms" << std::endl;
    std::cout << "Avg Inference Time: " << avg_time << " ms / frame" << std::endl;
    std::cout << "Estimated FPS: " << 1000.0f / avg_time << std::endl;

    return 0;
}
