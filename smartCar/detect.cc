#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <net.h>

struct Detection {
    cv::Rect2f box;
    int label;
    float score;
};

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
    for (int i = 0; i < n; i++) {
        areas[i] = dets[i].box.area();
    }

    for (int i = 0; i < n; i++) {
        const Detection& a = dets[i];
        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++) {
            const Detection& b = dets[picked[j]];
            const float inter_area = intersection_area(a, b);
            const float union_area = areas[i] + areas[picked[j]] - inter_area;
            if (inter_area / union_area > nms_threshold) {
                keep = 0;
                break;
            }
        }
        if (keep) picked.push_back(i);
    }
}

static bool load_model(ncnn::Net& net, const std::string& param_path, const std::string& bin_path)
{
    net.opt.use_vulkan_compute = false;
    net.opt.num_threads = 4;

    if (net.load_param(param_path.c_str()) != 0) {
        std::cerr << "Failed to load param: " << param_path << std::endl;
        return false;
    }
    if (net.load_model(bin_path.c_str()) != 0) {
        std::cerr << "Failed to load bin: " << bin_path << std::endl;
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
    in.substract_mean_normalize(nullptr, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    ex.input("in0", in);

    ncnn::Mat out;
    if (ex.extract("out0", out) != 0) {
        return {};
    }

    const int num_anchors = out.h;
    const int feat_dim = out.w;
    if (feat_dim < 5) {
        return {};
    }

    const int num_classes = feat_dim - 4;
    const float scale_x = (float)img_w / (float)input_size;
    const float scale_y = (float)img_h / (float)input_size;

    std::vector<Detection> proposals;
    proposals.reserve(num_anchors / 2);

    for (int i = 0; i < num_anchors; i++) {
        const float* row = out.row(i);
        const float x1 = row[0] * scale_x;
        const float y1 = row[1] * scale_y;
        const float x2 = row[2] * scale_x;
        const float y2 = row[3] * scale_y;

        int best_label = -1;
        float best_score = 0.f;
        for (int c = 0; c < num_classes; c++) {
            const float s = row[4 + c];
            if (s > best_score) {
                best_score = s;
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
    for (int c = 0; c < num_classes; c++) {
        std::vector<Detection> class_dets;
        for (const auto& d : proposals) {
            if (d.label == c) class_dets.push_back(d);
        }
        if (class_dets.empty()) continue;

        std::vector<int> picked;
        nms_sorted_bboxes(class_dets, picked, nms_thresh);
        for (int idx : picked) {
            results.push_back(class_dets[idx]);
        }
    }

    return results;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_video> <output_video> [score_thresh] [nms_thresh] [input_size]\n";
        return 1;
    }

    const std::string input_video = argv[1];
    const std::string output_video = argv[2];
    const float score_thresh = argc > 3 ? std::stof(argv[3]) : 0.35f;
    const float nms_thresh = argc > 4 ? std::stof(argv[4]) : 0.45f;
    const int input_size = argc > 5 ? std::stoi(argv[5]) : 320;

    const std::string param_path = "./model.ncnn.param";
    const std::string bin_path = "./model.ncnn.bin";

    ncnn::Net net;
    if (!load_model(net, param_path, bin_path)) {
        return 2;
    }

    cv::VideoCapture cap(input_video);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open input video: " << input_video << std::endl;
        return 3;
    }

    const int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 1.0) fps = 25.0;

    cv::VideoWriter writer(
        output_video,
        cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
        fps,
        cv::Size(width, height));

    if (!writer.isOpened()) {
        std::cerr << "Failed to open output video: " << output_video << std::endl;
        return 4;
    }

    int frame_id = 0;
    cv::Mat frame;
    while (cap.read(frame)) {
        std::vector<Detection> dets = infer_one_frame(net, frame, input_size, score_thresh, nms_thresh);

        for (const auto& d : dets) {
            cv::rectangle(frame, d.box, cv::Scalar(0, 255, 0), 2);
            char text[64];
            std::snprintf(text, sizeof(text), "cls:%d %.2f", d.label, d.score);
            cv::putText(frame, text, cv::Point((int)d.box.x, std::max(12, (int)d.box.y - 5)),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        }

        writer.write(frame);

        if (frame_id % 30 == 0) {
            std::cout << "processed frame: " << frame_id << " detections: " << dets.size() << std::endl;
        }
        frame_id++;
    }

    std::cout << "Done. Total frames: " << frame_id << " output: " << output_video << std::endl;
    return 0;
}
