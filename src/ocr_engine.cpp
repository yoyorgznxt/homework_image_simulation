#include "ocr_engine.h"
#include <regex>
#include <algorithm>
#include <iostream>

OCREngine::OCREngine(const std::string& language) : language_(language) {
    tess_api_ = std::make_unique<tesseract::TessBaseAPI>();
}

OCREngine::~OCREngine() {
    if (tess_api_) tess_api_->End();
}

bool OCREngine::initialize() {
    if (tess_api_->Init(nullptr, language_.c_str())) {
        std::cerr << "Tesseract 初始化失败" << std::endl;
        return false;
    }
    tess_api_->SetPageSegMode(tesseract::PSM_AUTO);
    return true;
}

std::vector<TextRegion> OCREngine::detectText(const cv::Mat& image) {
    std::vector<TextRegion> regions;
    Pix* pix = nullptr;
    if (image.channels() == 3) {
        cv::Mat rgb;
        cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
        pix = pixCreate(image.cols, image.rows, 32);
        for (int y = 0; y < image.rows; y++) {
            for (int x = 0; x < image.cols; x++) {
                cv::Vec3b pixel = rgb.at<cv::Vec3b>(y, x);
                uint32_t val = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
                pixSetPixel(pix, x, y, val);
            }
        }
    } else {
        pix = pixCreate(image.cols, image.rows, 8);
        for (int y = 0; y < image.rows; y++)
            for (int x = 0; x < image.cols; x++)
                pixSetPixel(pix, x, y, image.at<uchar>(y, x));
    }
    
    tess_api_->SetImage(pix);
    
    Boxa* boxes = tess_api_->GetComponentImages(tesseract::RIL_TEXTLINE, true, nullptr, nullptr);
    if (boxes) {
        int n = boxaGetCount(boxes);
        for (int i = 0; i < n; i++) {
            Box* box = boxaGetBox(boxes, i, L_CLONE);
            if (!box) continue;
            int bx, by, bw, bh;
            boxGetGeometry(box, &bx, &by, &bw, &bh);
            tess_api_->SetRectangle(bx, by, bw, bh);
            char* text = tess_api_->GetUTF8Text();
            if (text && strlen(text) > 0) {
                std::string str(text);
                str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                size_t p = str.find_last_not_of(" \t\r");
                if (p != std::string::npos) str.erase(p + 1);
                
                if (!str.empty()) {
                    TextRegion region;
                    region.text = str;
                    region.bbox = cv::Rect(bx, by, bw, bh);
                    region.confidence = tess_api_->MeanTextConf() / 100.0f;
                    regions.push_back(region);
                }
                delete[] text;
            }
            boxDestroy(&box);
        }
        boxaDestroy(&boxes);
    }
    
    Boxa* wordBoxes = tess_api_->GetComponentImages(tesseract::RIL_WORD, true, nullptr, nullptr);
    if (wordBoxes) {
        int n = boxaGetCount(wordBoxes);
        for (int i = 0; i < n; i++) {
            Box* box = boxaGetBox(wordBoxes, i, L_CLONE);
            if (!box) continue;
            int bx, by, bw, bh;
            boxGetGeometry(box, &bx, &by, &bw, &bh);
            if (bw < 15 || bh < 8) { boxDestroy(&box); continue; }
            tess_api_->SetRectangle(bx, by, bw, bh);
            char* text = tess_api_->GetUTF8Text();
            if (text && strlen(text) > 0) {
                std::string str(text);
                str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                size_t p = str.find_last_not_of(" \t\r");
                if (p != std::string::npos) str.erase(p + 1);
                
                if (!str.empty() && str.length() <= 8) {
                    TextRegion region;
                    region.text = str;
                    region.bbox = cv::Rect(bx, by, bw, bh);
                    region.confidence = tess_api_->MeanTextConf() / 100.0f;
                    regions.push_back(region);
                }
                delete[] text;
            }
            boxDestroy(&box);
        }
        boxaDestroy(&wordBoxes);
    }
    
    pixDestroy(&pix);
    return regions;
}

bool OCREngine::isQuestionNumber(const std::string& text, int& mainNum, int& subNum, std::string& subLabel) {
    mainNum = 0;
    subNum = 0;
    subLabel = "";
    
    // 匹配 "主题号.子题号" 如 "31.(1)" "31.①" "31.一"
    std::regex pattern1(R"(^\s*(\d{1,3})\s*[.．、]\s*(\(?\d+\)?|[①②③④⑤⑥⑦⑧⑨⑩]|[一二三四五六七八九十]))");
    std::smatch m1;
    if (std::regex_search(text, m1, pattern1)) {
        mainNum = std::stoi(m1[1].str());
        std::string sub = m1[2].str();
        subLabel = sub;
        if (sub == "①") subNum = 1; else if (sub == "②") subNum = 2;
        else if (sub == "③") subNum = 3; else if (sub == "④") subNum = 4;
        else if (sub == "⑤") subNum = 5; else if (sub == "⑥") subNum = 6;
        else if (sub == "⑦") subNum = 7; else if (sub == "⑧") subNum = 8;
        else if (sub == "一") subNum = 1; else if (sub == "二") subNum = 2;
        else if (sub == "三") subNum = 3; else if (sub == "四") subNum = 4;
        else { try { subNum = std::stoi(sub); } catch(...) { subNum = 0; } }
        return true;
    }
    
    // 匹配独立子题号: "(1)" "①" "一."
    std::regex pattern2(R"(^\s*(\(?\d+\)?)\s*[.．、]?\s*$)");
    std::regex pattern3(R"(^\s*([①②③④⑤⑥⑦⑧⑨⑩])\s*$)");
    std::regex pattern4(R"(^\s*([一二三四五六七八九十])\s*[.．、]?\s*$)");
    
    std::smatch m2;
    if (std::regex_match(text, m2, pattern2)) {
        std::string s = m2[1].str();
        if (s[0] == '(') s = s.substr(1, s.length()-2);
        try { subNum = std::stoi(s); } catch(...) { subNum = 0; }
        subLabel = "(" + std::to_string(subNum) + ")";
        mainNum = 0;
        return true;
    }
    if (std::regex_match(text, m2, pattern3)) {
        std::string s = m2[1].str();
        if (s=="①") subNum=1; else if (s=="②") subNum=2; else if (s=="③") subNum=3;
        else if (s=="④") subNum=4; else if (s=="⑤") subNum=5; else if (s=="⑥") subNum=6;
        subLabel = s;
        mainNum = 0;
        return true;
    }
    if (std::regex_match(text, m2, pattern4)) {
        std::string s = m2[1].str();
        if (s=="一") subNum=1; else if (s=="二") subNum=2; else if (s=="三") subNum=3;
        else if (s=="四") subNum=4; else if (s=="五") subNum=5; else if (s=="六") subNum=6;
        subLabel = s;
        mainNum = 0;
        return true;
    }
    
    // 匹配主题号: "31." "31," "31、"
    std::regex pattern5(R"(^\s*(\d{1,3})\s*[.．、,，)）])");
    if (std::regex_search(text, m2, pattern5)) {
        mainNum = std::stoi(m2[1].str());
        return true;
    }
    
    return false;
}


std::vector<QuestionInfo> OCREngine::extractQuestions(const cv::Mat& image) {
    std::vector<QuestionInfo> questions;
    auto regions = detectText(image);
    
    std::cout << "OCR 识别到 " << regions.size() << " 个文本区域" << std::endl;
    
    float avgTextHeight = 0;
    int count = 0;
    for (const auto& r : regions) {
        if (r.bbox.height > 10 && r.bbox.height < 200) {
            avgTextHeight += r.bbox.height;
            count++;
        }
    }
    if (count > 0) avgTextHeight /= count;
    std::cout << "平均文字高度: " << avgTextHeight << std::endl;
    
    int currentMain = 0;
    
    for (const auto& region : regions) {
        std::cout << "  '" << region.text << "' [" << region.bbox.x << "," << region.bbox.y 
                  << " " << region.bbox.width << "x" << region.bbox.height << "]" << std::endl;
        
        int mainNum = 0, subNum = 0;
        std::string subLabel;
        
        if (isQuestionNumber(region.text, mainNum, subNum, subLabel)) {
            QuestionInfo q;
            q.text = region.text;
            q.bbox = region.bbox;
            q.textHeight = region.bbox.height;
            if (q.textHeight < 15 || q.textHeight > 150) q.textHeight = (int)avgTextHeight;
            
            if (mainNum > 0) {
                currentMain = mainNum;
                q.number = mainNum;
                q.subNumber = subNum;
                q.subLabel = subLabel;
                std::cout << "  => 主题号 " << q.number;
                if (subNum > 0) std::cout << " 子题 " << subLabel;
                std::cout << " 高度=" << q.textHeight << std::endl;
            } else if (subNum > 0) {
                q.number = currentMain;
                q.subNumber = subNum;
                q.subLabel = subLabel;
                std::cout << "  => 子题号 " << subLabel << " (属于题号 " << currentMain << ") 高度=" << q.textHeight << std::endl;
            }
            
            questions.push_back(q);
        }
    }
    
    std::sort(questions.begin(), questions.end(),
              [](const QuestionInfo& a, const QuestionInfo& b) {
                  if (a.number != b.number) return a.number < b.number;
                  return a.subNumber < b.subNumber;
              });
    
    return questions;
}
