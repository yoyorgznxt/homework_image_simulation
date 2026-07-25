#ifndef EXAM_FILLER_H
#define EXAM_FILLER_H
#include <string>
#include <map>
#include "ocr_engine.h"
#include "image_processor.h"
#include "text_renderer.h"

class ExamFiller {
public:
    ExamFiller();
    bool loadAnswers(const std::string& path);
    bool loadFont(const std::string& path);
    bool process(const std::string& image_path, const std::string& output_path);
    void setDebug(bool debug);
private:
    std::map<int, std::string> answers_;
    std::unique_ptr<OCREngine> ocr_;
    std::unique_ptr<ImageProcessor> processor_;
    std::unique_ptr<TextRenderer> renderer_;
    bool debug_mode_;
};
#endif
