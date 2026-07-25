#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct LineInfo {
    cv::Point start;
    cv::Point end;
    cv::Rect bbox;
    int length;
    float angle;
};

class ImageProcessor {
public:
    cv::Mat loadImage(const std::string& path);
    bool saveImage(const std::string& path, const cv::Mat& image);
    cv::Mat preprocess(const cv::Mat& image);
    cv::Mat preprocessForOCR(const cv::Mat& image);
    std::vector<LineInfo> detectHorizontalLines(const cv::Mat& image, int min_length = 50, bool debug = false);
    LineInfo findLongestLine(const std::vector<LineInfo>& lines);
};
#endif
