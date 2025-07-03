#include "../native_opencv.h"
#include <opencv2/opencv.hpp>

extern "C" {
    FUNCTION_ATTRIBUTE
    void cannyEdgeDetectorV2(
            char *inputPath,
            char *outputPath,
            int gaussianKernelSize,  // Kích thước kernel Gaussian (nên là số lẻ)
            float gaussianSigma,     // Độ mờ Gaussian
            float cannyLowThresh,    // Ngưỡng dưới Canny
            float cannyHighThresh,   // Ngưỡng trên Canny
            int edgeR,               // Màu đỏ của viền (0-255)
            int edgeG,               // Màu xanh lá của viền (0-255)
            int edgeB,               // Màu xanh dương của viền (0-255)
            int edgeA,               // Độ trong suốt viền (0-255)
            int dilationSize         // Độ dày viền (0 để tắt)
    ) {
        // Đọc ảnh đầu vào
        cv::Mat image = cv::imread(inputPath, cv::IMREAD_UNCHANGED);
        if (image.empty()) {
            std::cerr << "Error load image" << std::endl;
            return;
        }

        // Chuyển sang ảnh xám nếu cần
        cv::Mat gray;
        if (image.channels() == 3) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        // Đảm bảo kernel size là số lẻ dương
        gaussianKernelSize = std::max(1, gaussianKernelSize);
        if (gaussianKernelSize % 2 == 0) gaussianKernelSize++;

        // Làm mờ ảnh
        cv::GaussianBlur(gray, gray,
                         cv::Size(gaussianKernelSize, gaussianKernelSize),
                         gaussianSigma);

        // Phát hiện biên Canny
        cv::Mat edges;
        cv::Canny(gray, edges, cannyLowThresh, cannyHighThresh);

        // Làm dày viền nếu cần
        if (dilationSize > 0) {
            cv::Mat kernel = cv::getStructuringElement(
                    cv::MORPH_RECT,
                    cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1)
            );
            cv::dilate(edges, edges, kernel);
        }

        // Tạo ảnh output với nền trong suốt
        cv::Mat result(image.rows, image.cols, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        // Đổ màu viền theo tham số
        for (int i = 0; i < edges.rows; ++i) {
            for (int j = 0; j < edges.cols; ++j) {
                if (edges.at<uchar>(i, j) > 0) {
                    result.at<cv::Vec4b>(i, j) = cv::Vec4b(
                            edgeB, edgeG, edgeR, edgeA
                    );
                }
            }
        }

        // Lưu ảnh kết quả
        cv::imwrite(outputPath, result);
    }
}