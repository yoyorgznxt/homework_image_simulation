#include "text_renderer.h"
#include <iostream>
#include <cmath>

TextRenderer::TextRenderer() : font_loaded_(false) {
    if (FT_Init_FreeType(&ft_library_)) throw std::runtime_error("FreeType 初始化失败");
}

TextRenderer::~TextRenderer() {
    if (font_loaded_) FT_Done_Face(ft_face_);
    FT_Done_FreeType(ft_library_);
}

bool TextRenderer::loadFont(const std::string& font_path) {
    if (FT_New_Face(ft_library_, font_path.c_str(), 0, &ft_face_)) {
        std::cerr << "无法加载字体: " << font_path << std::endl;
        return false;
    }
    font_loaded_ = true;
    return true;
}

int TextRenderer::calculateFontSize(int refHeight) {
    if (refHeight > 0) {
        int size = (int)(refHeight * 1.2);
        if (size < 14) size = 14;
        if (size > 240) size = 240;
        return size;
    }
    return 24;
}

void TextRenderer::drawTextOnLine(cv::Mat& image, const std::string& text, 
                                   const cv::Point& start, const cv::Point& end, int refHeight) {
    if (!font_loaded_ || text.empty()) return;
    
    int font_size = calculateFontSize(refHeight);
    std::cout << "    字体大小: " << font_size << std::endl;
    
    FT_Set_Pixel_Sizes(ft_face_, 0, font_size);
    
    int total_width = 0;
    int ascender = 0;
    for (char c : text) {
        if (FT_Load_Char(ft_face_, c, FT_LOAD_RENDER)) continue;
        total_width += ft_face_->glyph->advance.x >> 6;
        if (ft_face_->glyph->bitmap_top > ascender) 
            ascender = ft_face_->glyph->bitmap_top;
    }
    
    if (total_width == 0) return;
    
    // 计算线段角度
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float angle = std::atan2(dy, dx);
    
    // 在斜线上居中放置
    float mid_ratio = 0.5f;
    int center_x = start.x + (int)(dx * mid_ratio);
    int center_y = start.y + (int)(dy * mid_ratio);
    
    int x_start = center_x - total_width / 2;
    int y_baseline = center_y - 4;
    
    if (x_start < 0) x_start = 0;
    if (x_start + total_width > image.cols) x_start = image.cols - total_width - 2;
    if (y_baseline - ascender < 0) y_baseline = ascender + 2;
    
    // 沿斜线逐个字符绘制
    int x_offset = x_start;
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    
    for (char c : text) {
        if (FT_Load_Char(ft_face_, c, FT_LOAD_RENDER)) continue;
        
        FT_GlyphSlot glyph = ft_face_->glyph;
        FT_Bitmap* bitmap = &glyph->bitmap;
        
        int char_advance = glyph->advance.x >> 6;
        
        for (unsigned int row = 0; row < bitmap->rows; row++) {
            for (unsigned int col = 0; col < bitmap->width; col++) {
                int dx_char = x_offset + glyph->bitmap_left + col - center_x;
                int dy_char = y_baseline - glyph->bitmap_top + row - center_y;
                
                int px = center_x + (int)(dx_char * cos_a - dy_char * sin_a);
                int py = center_y + (int)(dx_char * sin_a + dy_char * cos_a);
                
                if (px >= 0 && px < image.cols && py >= 0 && py < image.rows) {
                    unsigned char alpha = bitmap->buffer[row * bitmap->pitch + col];
                    if (alpha > 128) {
                        cv::Vec3b& pixel = image.at<cv::Vec3b>(py, px);
                        pixel[0] = 0;
                        pixel[1] = 0;
                        pixel[2] = 0;
                    }
                }
            }
        }
        x_offset += char_advance;
    }
}
