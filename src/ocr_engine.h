#ifndef OCR_ENGINE_H
#define OCR_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>

struct TextRegion {
    std::string text;
    cv::Rect bbox;
    float confidence;
};

struct QuestionInfo {
    int number;
    int subNumber;
    std::string subLabel;
    std::string text;
    cv::Rect bbox;
    int textHeight;
};

class OCREngine {
public:
    OCREngine(const std::string& language = "chi_sim+eng");
    ~OCREngine();
    bool initialize();
    std::vector<TextRegion> detectText(const cv::Mat& image);
    std::vector<QuestionInfo> extractQuestions(const cv::Mat& image);
private:
    std::unique_ptr<tesseract::TessBaseAPI> tess_api_;
    std::string language_;
    bool isQuestionNumber(const std::string& text, int& mainNum, int& subNum, std::string& subLabel);
};
#endif
