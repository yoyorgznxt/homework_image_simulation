#include <iostream>
#include <string>
#include <cstring>
#include "exam_filler.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "用法: " << argv[0] << " <image> <answers> <font> [output] [--debug]" << std::endl;
        return 1;
    }
    
    std::string image_path = argv[1];
    std::string answers_path = argv[2];
    std::string font_path = argv[3];
    std::string output_path = "filled_exam.jpg";
    bool debug = false;
    
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug = true;
        } else {
            output_path = argv[i];
        }
    }
    
    ExamFiller filler;
    filler.setDebug(debug);
    
    if (!filler.loadAnswers(answers_path)) return 1;
    if (!filler.loadFont(font_path)) return 1;
    if (!filler.process(image_path, output_path)) return 1;
    
    std::cout << "输出: " << output_path << std::endl;
    return 0;
}
