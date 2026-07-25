## 纯Vibe Coding，bug满天飞，修bug中
# 自动化作业答案填充软件

自动识别作业图片中的题号和填空横线，将答案以手写字体填入横线上方。

## 功能

- OCR 识别题号（支持主题号 + 子题号：①②③、(1)(2)(3)、一.二.三.）
- 霍夫变换检测横线（支持 ±15° 斜线）
- 手写字体渲染，字号自适应题目文字大小
- 答案沿斜线方向绘制

## 编译环境

Debian 13 x64

## 依赖安装

```bash
sudo apt install -y build-essential cmake pkg-config \
    libopencv-dev libtesseract-dev libleptonica-dev \
    libfreetype6-dev tesseract-ocr-chi-sim tesseract-ocr-eng
```

编译

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

使用

```bash
./ExamFiller <试卷图片> <答案文件> <字体文件> [输出图片] [--debug]
```

示例：

```bash
./ExamFiller exam.jpg answers.txt handwriting.ttf output.jpg --debug
```

参数说明：

· 试卷图片：jpg/png 格式
· 答案文件：答案配置（格式见下文）
· 字体文件：.ttf 手写字体
· 输出图片：可选，默认 filled_exam.jpg
· --debug：可选，输出调试图片 debug_lines.png

答案文件格式

```
# 主题号答案
31=主题答案

# 子题号答案（主题号-子题号）
31-1=第一小题答案
31-2=第二小题答案
31-①=圈1小题答案
31-一=小题一答案
```

· # 开头为注释
· 主题号：题号=答案
· 子题号：主题号-子题号=答案

技术方案

1. OCR：Tesseract 5.x，PSM_AUTO 模式，文本行+单词两级识别
2. 预处理：自适应二值化 + 形态学去噪 + 2倍放大
3. 横线检测：霍夫线检测，过滤 ±15° 以外的线段
4. 字体渲染：FreeType 2，沿斜线旋转绘制，字号=题号高度×1.2
   HEREDOC_END