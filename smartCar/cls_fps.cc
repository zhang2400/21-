#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <net.h>

static int parse_int_arg(int argc, char** argv, int index, int default_value)
{
    // 读取第 index 个命令行整数参数；如果没传，就使用 default_value。
    if (argc <= index) return default_value;
    return std::atoi(argv[index]);
}

static const char* parse_str_arg(int argc, char** argv, int index, const char* default_value)
{
    // 读取第 index 个命令行字符串参数；常用于模型路径。
    if (argc <= index) return default_value;
    return argv[index];
}

static void fill_dummy_input(ncnn::Mat& input)
{
    // 构造一张固定的假输入图像，用来单独测试模型推理速度。
    // 这里不读摄像头、不读图片，所以测出来的是纯模型推理 FPS。
    for (int q = 0; q < input.c; q++) {
        float* ptr = input.channel(q);
        for (int i = 0; i < input.w * input.h; i++) {
            ptr[i] = ((i * 37 + q * 53) % 255) / 255.0f;
        }
    }
}

static bool run_once(ncnn::Net& net, const ncnn::Mat& input, ncnn::Mat& output)
{
    ncnn::Extractor ex = net.create_extractor();
    // light mode 会尽量复用/释放中间内存，嵌入式板子上通常更合适。
    ex.set_light_mode(true);

    // 输入节点名来自 cls.param 第一行 Input 层：
    // Input input 0 1 input
    // 如果以后模型换了，输入节点名要和新的 .param 文件保持一致。
    if (ex.input("input", input) != 0) {
        std::cerr << "Failed to set input node: input" << std::endl;
        return false;
    }

    // 输出节点名来自 cls.param 最后一层：
    // InnerProduct ... output
    // 如果以后模型换了，输出节点名也要对应修改。
    if (ex.extract("output", output) != 0) {
        std::cerr << "Failed to extract output node: output" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    // 命令行参数从 argv[1] 开始：
    // argv[1] loop_count   正式测速循环次数，越大平均值越稳定，默认 100。
    // argv[2] warmup_count 预热次数，不计入 FPS，默认 10。
    // argv[3] input_size   输入图像边长，默认 64，表示 64x64x3。
    // argv[4] num_threads  ncnn CPU 线程数，默认 1。
    // argv[5] param_path   cls.param 路径，默认 ./cls.param。
    // argv[6] bin_path     cls.bin 路径，默认 ./cls.bin。
    //
    // 示例：
    // ./cls_fps
    // ./cls_fps 500 50 64 1
    // ./cls_fps 500 50 64 2 ./cls.param ./cls.bin
    const int loop_count = std::max(1, parse_int_arg(argc, argv, 1, 100));
    const int warmup_count = std::max(0, parse_int_arg(argc, argv, 2, 10));
    const int input_size = std::max(1, parse_int_arg(argc, argv, 3, 160));
    const int num_threads = std::max(1, parse_int_arg(argc, argv, 4, 2));
    const char* param_path = parse_str_arg(argc, argv, 5, "./cls.param");
    const char* bin_path = parse_str_arg(argc, argv, 6, "./cls.bin");

    std::cout << "Usage: " << argv[0]
              << " [loop_count=100] [warmup_count=10] [input_size=64] [threads=1]"
              << " [param=./cls.param] [bin=./cls.bin]\n";

    ncnn::Net net;
    // 线程数会明显影响性能。ls2k0300 上建议分别测试 1、2、4，
    // 小模型不一定线程越多越快，以实际 FPS 为准。
    net.opt.num_threads = num_threads;
    net.opt.use_vulkan_compute = false;
    // 下面这些选项关闭 fp16 和 packing layout，偏向在普通 CPU 标量路径上稳定测速。
    // 如果以后换成支持更强 SIMD/向量优化的平台，可以尝试打开再对比 FPS。
    net.opt.use_fp16_packed = false;
    net.opt.use_fp16_storage = false;
    net.opt.use_fp16_arithmetic = false;
    net.opt.use_packing_layout = false;

    if (net.load_param(param_path) != 0) {
        std::cerr << "Failed to load param: " << param_path << std::endl;
        return 1;
    }
    if (net.load_model(bin_path) != 0) {
        std::cerr << "Failed to load bin: " << bin_path << std::endl;
        return 2;
    }

    // 创建模型输入。当前 cls.param 只声明了输入节点名，没有固定写死宽高，
    // 所以这里通过 input_size 控制输入大小。必须和训练/导出时使用的尺寸一致，
    // 否则 FPS 可以测，但分类结果可能没有意义。
    ncnn::Mat input(input_size, input_size, 3);
    fill_dummy_input(input);

    std::cout << "Model: " << param_path << " / " << bin_path << '\n'
              << "Input: " << input_size << "x" << input_size << "x3"
              << ", threads: " << num_threads
              << ", warmup: " << warmup_count
              << ", loops: " << loop_count << std::endl;

    ncnn::Mat output;
    // 预热阶段不计入最终 FPS，主要用于稳定 CPU 频率、缓存和首次执行开销。
    for (int i = 0; i < warmup_count; i++) {
        if (!run_once(net, input, output)) return 3;
    }

    double total_ms = 0.0;
    double min_ms = 1e30;
    double max_ms = 0.0;

    // 正式测速：每次创建 Extractor，输入同一张假图，统计单次推理耗时。
    for (int i = 0; i < loop_count; i++) {
        const auto begin = std::chrono::steady_clock::now();
        if (!run_once(net, input, output)) return 4;
        const auto end = std::chrono::steady_clock::now();

        const double elapsed_ms = std::chrono::duration<double, std::milli>(end - begin).count();
        total_ms += elapsed_ms;
        min_ms = std::min(min_ms, elapsed_ms);
        max_ms = std::max(max_ms, elapsed_ms);
    }

    const double avg_ms = total_ms / loop_count;
    // FPS = 1000 ms / 平均每帧耗时。
    const double fps = 1000.0 / avg_ms;

    std::cout << std::fixed << std::setprecision(3)
              << "Output shape: w=" << output.w
              << " h=" << output.h
              << " c=" << output.c << '\n'
              << "Total: " << total_ms << " ms\n"
              << "Average: " << avg_ms << " ms/frame\n"
              << "Min: " << min_ms << " ms, Max: " << max_ms << " ms\n"
              << "FPS: " << fps << std::endl;

    return 0;
}
