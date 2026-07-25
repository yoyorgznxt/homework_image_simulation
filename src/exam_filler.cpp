#include "exam_filler.h"
#include <fstream>
#include <iostream>
#include <cstring>

ExamFiller::ExamFiller() {
    ocr_ = std::make_unique<OCREngine>("chi_sim+eng");
    processor_ = std::make_unique<ImageProcessor>();
    renderer_ = std::make_unique<TextRenderer>();
    debug_mode_ = false;
}

bool ExamFiller::loadAnswers(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) { std::cerr << "无法打开: " << path << std::endl; return false; }
    answers_.clear();
    std::string line;
    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        while (!line.empty() && line.front() == ' ') line.erase(0, 1);
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            try {
                int num = std::stoi(line.substr(0, pos));
                std::string ans = line.substr(pos + 1);
                while (!ans.empty() && ans.front() == ' ') ans.erase(0, 1);
                while (!ans.empty() && ans.back() == ' ') ans.pop_back();
                answers_[num] = ans;
            } catch (...) { std::cerr << "解析错误: " << line << std::endl; }
        }
    }
    std::cout << "已加载 " << answers_.size() << " 条答案" << std::endl;
    return true;
}

bool ExamFiller::loadFont(const std::string& path) { return renderer_->loadFont(path); }

void ExamFiller::setDebug(bool debug) { debug_mode_ = debug; }

bool ExamFiller::process(const std::string& image_path, const std::string& output_path) {
    try {
        std::cout << "正在加载图片..." << std::endl;
        cv::Mat image = processor_->loadImage(image_path);
        
        std::cout << "OCR 预处理中..." << std::endl;
        cv::Mat ocr_image = processor_->preprocessForOCR(image);
        float scale_x = (float)ocr_image.cols / image.cols;
        float scale_y = (float)ocr_image.rows / image.rows;
        
        if (!ocr_->initialize()) { std::cerr << "OCR 初始化失败" << std::endl; return false; }
        
        std::cout << "正在识别题号..." << std::endl;
        auto questions = ocr_->extractQuestions(ocr_image);
        
        for (auto& q : questions) {
            q.bbox.x = (int)(q.bbox.x / scale_x);
            q.bbox.y = (int)(q.bbox.y / scale_y);
            q.bbox.width = (int)(q.bbox.width / scale_x);
            q.bbox.height = (int)(q.bbox.height / scale_y);
            q.textHeight = (int)(q.textHeight / scale_y);
        }
        std::cout << "识别到 " << questions.size() << " 个题号" << std::endl;
        
        std::cout << "正在检测横线 (霍夫变换, 角度<=15度)..." << std::endl;
        auto lines = processor_->detectHorizontalLines(image, 40, debug_mode_);
        std::cout << "找到 " << lines.size() << " 条横线" << std::endl;
        
        LineInfo longest = processor_->findLongestLine(lines);
        if (longest.length == 0) {
            std::cerr << "未检测到横线!" << std::endl;
            return false;
        }
        std::cout << "最长横线: 角度=" << longest.angle << " 度, 长度=" << longest.length << std::endl;
        
        int filled = 0;
        float total_len = (float)longest.length;
        float spacing = total_len / std::max(1, (int)questions.size());
        
        for (size_t i = 0; i < questions.size(); i++) {
            const auto& q = questions[i];
            auto it = answers_.find(q.number);
            if (it == answers_.end()) { 
                std::cout << "无答案: 题号 " << q.number << std::endl; 
                continue; 
            }
            
            float ratio_start = i * spacing / total_len;
            float ratio_end = (i + 1) * spacing / total_len;
            
            cv::Point seg_start(
                longest.start.x + (int)((longest.end.x - longest.start.x) * ratio_start),
                longest.start.y + (int)((longest.end.y - longest.start.y) * ratio_start)
            );
            cv::Point seg_end(
                longest.start.x + (int)((longest.end.x - longest.start.x) * ratio_end),
                longest.start.y + (int)((longest.end.y - longest.start.y) * ratio_end)
            );
            
            std::cout << "填入 题号 " << q.number << ": " << it->second << std::endl;
            renderer_->drawTextOnLine(image, it->second, seg_start, seg_end, q.textHeight);
            filled++;
        }
        
        std::cout << "保存中..." << std::endl;
        processor_->saveImage(output_path, image);
        std::cout << "完成! " << filled << "/" << questions.size() << std::endl;
        return true;
    } catch (const std::exception& e) { std::cerr << "错误: " << e.what() << std::endl; return false; }
}
