#include "../native_opencv.h"
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
FUNCTION_ATTRIBUTE
void removeWhiteBg(char* inputPath, char* outputPath, int whiteThreshold) {
    // Đọc ảnh đầu vào
    cv::Mat image = cv::imread(inputPath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Không thể đọc ảnh: " << inputPath << std::endl;
        return;
    }

    // Kiểm tra ngưỡng hợp lệ
    if (whiteThreshold < 0 || whiteThreshold > 255) {
        std::cerr << "Ngưỡng phải nằm trong khoảng [0, 255]" << std::endl;
        return;
    }

    // Tạo mask phát hiện pixel trắng và gần trắng
    cv::Mat mask;
    cv::inRange(image,
                cv::Scalar(whiteThreshold, whiteThreshold, whiteThreshold),
                cv::Scalar(255, 255, 255),
                mask);

    // Đảo mask để các pixel trắng thành 0 (trong suốt)
    cv::bitwise_not(mask, mask);

    // Xử lý morphology để làm sạch mask
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), 2);

    // Chuyển sang ảnh BGRA
    cv::Mat result;
    cv::cvtColor(image, result, cv::COLOR_BGR2BGRA);

    // Áp dụng mask vào kênh alpha
    cv::Mat channels[4];
    cv::split(result, channels);
    channels[3] = mask;
    cv::merge(channels, 4, result);

    // Lưu ảnh kết quả
    if (!cv::imwrite(outputPath, result)) {
        std::cerr << "Lỗi khi lưu ảnh: " << outputPath << std::endl;
    } else {
        std::cout << "Xử lý thành công với ngưỡng " << whiteThreshold
                  << ". Ảnh đã lưu: " << outputPath << std::endl;
    }
}
}