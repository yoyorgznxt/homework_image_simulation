#!/bin/bash
# install_deps.sh - Debian 13依赖安装脚本

echo "正在更新系统包列表..."
sudo apt update

echo "安装基础编译工具..."
sudo apt install -y build-essential cmake git pkg-config

echo "安装OpenCV..."
sudo apt install -y libopencv-dev libopencv-contrib-dev

echo "安装Tesseract OCR..."
sudo apt install -y tesseract-ocr libtesseract-dev libleptonica-dev
sudo apt install -y tesseract-ocr-chi-sim tesseract-ocr-chi-tra
sudo apt install -y tesseract-ocr-eng

echo "安装FreeType..."
sudo apt install -y libfreetype6-dev

echo "安装图像处理库..."
sudo apt install -y libpng-dev libjpeg-dev libtiff-dev

echo "验证安装..."

# 检查OpenCV
echo -n "检查OpenCV... "
if pkg-config --exists opencv4; then
    echo "已安装 (版本: $(pkg-config --modversion opencv4))"
    echo "OpenCV头文件路径: $(pkg-config --cflags opencv4)"
    echo "OpenCV库路径: $(pkg-config --libs opencv4)"
elif [ -f "/usr/lib/x86_64-linux-gnu/cmake/opencv4/OpenCVConfig.cmake" ]; then
    echo "已安装 (通过cmake配置文件找到)"
elif [ -f "/usr/share/opencv4/OpenCVConfig.cmake" ]; then
    echo "已安装 (在/usr/share中找到)"
else
    echo "未找到"
    echo "尝试手动查找OpenCV配置文件..."
    find /usr -name "OpenCVConfig.cmake" 2>/dev/null
fi

# 检查Tesseract
echo -n "检查Tesseract... "
if pkg-config --exists tesseract; then
    echo "已安装 (版本: $(pkg-config --modversion tesseract))"
    echo "Tesseract头文件: $(pkg-config --cflags tesseract)"
    echo "Tesseract库: $(pkg-config --libs tesseract)"
else
    echo "未通过pkg-config找到"
    echo "检查可执行文件..."
    which tesseract
    tesseract --version 2>/dev/null | head -n1
fi

# 检查FreeType2
echo -n "检查FreeType2... "
if pkg-config --exists freetype2; then
    echo "已安装 (版本: $(pkg-config --modversion freetype2))"
else
    echo "未找到"
fi

# 检查Leptonica
echo -n "检查Leptonica... "
if pkg-config --exists lept; then
    echo "已安装 (版本: $(pkg-config --modversion lept))"
else
    echo "未通过pkg-config找到"
    echo "尝试手动查找..."
    find /usr -name "leptonica" -type d 2>/dev/null
fi

echo ""
echo "依赖检查完成。"
echo "如果所有组件都已安装，可以开始编译项目。"