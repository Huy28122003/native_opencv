#include "../native_opencv.h"
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
FUNCTION_ATTRIBUTE
void sketch(char* path, char* output) {
    // Đọc ảnh và kiểm tra
    cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Không thể đọc ảnh: " << path << std::endl;
        return;
    }

    // Chuyển ảnh sang grayscale
    cv::Mat gray_image;
    cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);

    // Đảo màu ảnh xám
    cv::Mat invert;
    cv::bitwise_not(gray_image, invert);

    // Làm mờ với kernel nhỏ hơn
    cv::Mat blur;
    cv::GaussianBlur(invert, blur, cv::Size(15,15), 0);

    // Đảo màu ảnh làm mờ
    cv::Mat inverted_blur;
    cv::bitwise_not(blur, inverted_blur);

    // Tạo ảnh sketch
    cv::Mat sketch;
    cv::divide(gray_image, inverted_blur, sketch, 256.0);

    // Giảm ngưỡng threshold để bảo toàn nhiều chi tiết hơn
    cv::Mat sketch_thresholded;
    cv::threshold(sketch, sketch_thresholded, 180, 255, cv::THRESH_BINARY);

    // Tạo phiên bản tối ưu cho biên
    cv::Mat edges;
    cv::Canny(sketch_thresholded, edges, 50, 150, 3);  // Giảm ngưỡng để phát hiện nhiều biên hơn

    // Áp dụng phép dãn (dilation) để kết nối các đường nét gần nhau
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
    cv::Mat dilated_edges;
    cv::dilate(edges, dilated_edges, kernel);

    // Áp dụng phép đóng (closing) để lấp đầy các khoảng trống nhỏ
    cv::Mat closed_edges;
    cv::morphologyEx(dilated_edges, closed_edges, cv::MORPH_CLOSE, kernel);

    // Tạo ảnh RGBA
    cv::Mat result;
    cv::cvtColor(sketch_thresholded, result, cv::COLOR_GRAY2BGRA);

    // Áp dụng mask alpha
    cv::Mat channels[4];
    cv::split(result, channels);
    channels[3] = closed_edges;  // Sử dụng closed_edges thay vì edges ban đầu
    cv::merge(channels, 4, result);

    // Lưu ảnh
    if (!cv::imwrite(output, result)) {
        std::cerr << "Lỗi khi lưu ảnh: " << output << std::endl;
    } else {
        std::cout << "Ảnh đã lưu: " << output << std::endl;
    }
}
}