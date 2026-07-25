cmake_minimum_required(VERSION 3.10)
project(ExamFiller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 必须使用pkg-config
find_package(PkgConfig REQUIRED)

# 查找OpenCV
pkg_check_modules(OpenCV REQUIRED opencv4)
message(STATUS "Found OpenCV ${OpenCV_VERSION}")
message(STATUS "  Include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${OpenCV_LIBRARIES}")
message(STATUS "  Library dirs: ${OpenCV_LIBRARY_DIRS}")

# 查找Tesseract
pkg_check_modules(Tesseract REQUIRED tesseract)
message(STATUS "Found Tesseract ${Tesseract_VERSION}")
message(STATUS "  Include: ${Tesseract_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${Tesseract_LIBRARIES}")

# 查找FreeType2
pkg_check_modules(FREETYPE REQUIRED freetype2)
message(STATUS "Found FreeType2 ${FREETYPE_VERSION}")
message(STATUS "  Include: ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${FREETYPE_LIBRARIES}")

# 查找Leptonica
pkg_check_modules(LEPTONICA REQUIRED lept)
message(STATUS "Found Leptonica ${LEPTONICA_VERSION}")
message(STATUS "  Include: ${LEPTONICA_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${LEPTONICA_LIBRARIES}")

# 设置源文件
set(SOURCES
    src/main.cpp
    src/ocr_engine.cpp
    src/image_processor.cpp
    src/text_renderer.cpp
    src/exam_filler.cpp
)

set(HEADERS
    src/ocr_engine.h
    src/image_processor.h
    src/text_renderer.h
    src/exam_filler.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    ${Tesseract_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
)

# 设置库目录
target_link_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBRARY_DIRS}
    ${Tesseract_LIBRARY_DIRS}
    ${FREETYPE_LIBRARY_DIRS}
    ${LEPTONICA_LIBRARY_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBRARIES}
    ${Tesseract_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
)

# 添加编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${OpenCV_CFLAGS_OTHER}
    ${Tesseract_CFLAGS_OTHER}
    ${FREETYPE_CFLAGS_OTHER}
    ${LEPTONICA_CFLAGS_OTHER}
)

# 设置编译选项
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)

# Windows交叉编译设置
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    target_link_libraries(${PROJECT_NAME} -lws2_32)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)