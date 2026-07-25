#include "image_processor.h"
#include <iostream>
#include <limits>
#include <cmath>

cv::Mat ImageProcessor::loadImage(const std::string& path) {
    cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
    if (image.empty()) throw std::runtime_error("无法加载图片: " + path);
    return image;
}

bool ImageProcessor::saveImage(const std::string& path, const cv::Mat& image) {
    return cv::imwrite(path, image);
}

cv::Mat ImageProcessor::preprocess(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else gray = image.clone();
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, gray);
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
    return gray;
}

cv::Mat ImageProcessor::preprocessForOCR(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 3) cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else gray = image.clone();
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                          cv::THRESH_BINARY, 15, 8);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);
    cv::Mat enlarged;
    cv::resize(binary, enlarged, cv::Size(binary.cols * 2, binary.rows * 2), 0, 0, cv::INTER_CUBIC);
    return enlarged;
}

std::vector<LineInfo> ImageProcessor::detectHorizontalLines(const cv::Mat& image, int min_length, bool debug) {
    std::vector<LineInfo> lines;
    cv::Mat gray;
    if (image.channels() == 3) cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else gray = image.clone();
    
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(gray, gray);
    
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    
    // 调试图片在if外面声明
    cv::Mat debug_img;
    if (debug) {
        debug_img = image.clone();
    }
    
    std::vector<cv::Vec4i> houghLines;
    cv::HoughLinesP(binary, houghLines, 1, CV_PI / 180, 50, min_length, 10);
    
    if (debug) {
        std::cout << "  霍夫线: " << houghLines.size() << std::endl;
    }
    
    for (const auto& l : houghLines) {
        int x1 = l[0], y1 = l[1], x2 = l[2], y2 = l[3];
        
        float dx = x2 - x1;
        float dy = y2 - y1;
        float angle = std::atan2(dy, dx) * 180.0f / CV_PI;
        
        // 只保留±15度以内的线
        if (std::abs(angle) > 15.0f) continue;
        
        int length = (int)std::sqrt(dx*dx + dy*dy);
        if (length < min_length) continue;
        
        LineInfo line;
        line.start = cv::Point(x1, y1);
        line.end = cv::Point(x2, y2);
        line.length = length;
        line.angle = angle;
        line.bbox = cv::Rect(std::min(x1,x2), std::min(y1,y2), 
                             std::abs(x2-x1), std::abs(y2-y1) + 4);
        
        lines.push_back(line);
        
        if (debug) {
            std::cout << "    线段: (" << x1 << "," << y1 << ")->(" << x2 << "," << y2 
                      << ") 长度=" << length << " 角度=" << angle << std::endl;
            cv::line(debug_img, cv::Point(x1,y1), cv::Point(x2,y2), cv::Scalar(0,255,0), 2);
        }
    }
    
    if (debug) {
        cv::imwrite("debug_lines.png", debug_img);
        std::cout << "  调试: 已保存 debug_lines.png" << std::endl;
    }
    
    return lines;
}

LineInfo ImageProcessor::findLongestLine(const std::vector<LineInfo>& lines) {
    LineInfo best;
    int max_length = 0;
    for (const auto& line : lines) {
        if (line.length > max_length) {
            max_length = line.length;
            best = line;
        }
    }
    return best;
}
