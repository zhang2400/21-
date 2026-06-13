#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <net.h>

static int parse_int_arg(int argc, char** argv, int index, int default_value)
{
    if (argc <= index) return default_value;
    return std::atoi(argv[index]);
}

static const char* parse_str_arg(int argc, char** argv, int index, const char* default_value)
{
    if (argc <= index) return default_value;
    return argv[index];
}

static bool load_cls_model(ncnn::Net& net, const char* param_path, const char* bin_path, int threads)
{
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
    return true;
}

static bool classify_image(
    ncnn::Net& net,
    const cv::Mat& bgr,
    int input_size,
    std::vector<float>& scores)
{
    // cls.param 的输入节点名是 input，输出节点名是 output。
    // 这里把任意大小的图片缩放成 input_size x input_size x 3。
    ncnn::Mat input = ncnn::Mat::from_pixels_resize(
        bgr.data,
        ncnn::Mat::PIXEL_BGR2RGB,
        bgr.cols,
        bgr.rows,
        input_size,
        input_size);

    // 和常见分类模型预处理保持一致：像素值从 0~255 归一化到 0~1。
    // 如果你的训练时用了 mean/std，这里要改成对应的 mean_vals/norm_vals。
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

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <image_path> [input_size=64] [param=./cls.param] [bin=./cls.bin] [threads=1]\n";
        return 1;
    }

    const char* image_path = argv[1];
    const int input_size = std::max(1, parse_int_arg(argc, argv, 2, 64));
    const char* param_path = parse_str_arg(argc, argv, 3, "./cls.param");
    const char* bin_path = parse_str_arg(argc, argv, 4, "./cls.bin");
    const int threads = std::max(1, parse_int_arg(argc, argv, 5, 1));

    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Failed to read image: " << image_path << std::endl;
        return 2;
    }

    ncnn::Net net;
    if (!load_cls_model(net, param_path, bin_path, threads)) {
        return 3;
    }

    std::vector<float> scores;
    if (!classify_image(net, image, input_size, scores)) {
        return 4;
    }

    if (scores.empty()) {
        std::cerr << "Empty output." << std::endl;
        return 5;
    }

    const int top_id = (int)(std::max_element(scores.begin(), scores.end()) - scores.begin());
    const float top_score = scores[top_id];

    std::cout << "Image: " << image_path << '\n'
              << "Model: " << param_path << " / " << bin_path << '\n'
              << "Input size: " << input_size << "x" << input_size << "x3"
              << ", threads: " << threads << '\n'
              << "Class count: " << scores.size() << '\n';

    std::cout << std::fixed << std::setprecision(6);
    for (int i = 0; i < (int)scores.size(); i++) {
        std::cout << "class " << i << ": " << scores[i] << '\n';
    }

    std::cout << "Top1: class " << top_id << ", score: " << top_score << std::endl;

    return 0;
}
