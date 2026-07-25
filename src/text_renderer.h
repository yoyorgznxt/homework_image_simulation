#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H
#include <string>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <opencv2/opencv.hpp>

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();
    bool loadFont(const std::string& font_path);
    void drawTextOnLine(cv::Mat& image, const std::string& text, 
                        const cv::Point& start, const cv::Point& end, int refHeight = 0);
private:
    FT_Library ft_library_;
    FT_Face ft_face_;
    bool font_loaded_;
    int calculateFontSize(int refHeight);
};
#endif
