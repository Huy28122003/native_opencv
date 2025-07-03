#include "../native_opencv.h"
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
FUNCTION_ATTRIBUTE
void removeBg(char* inputPath, char* outputPath) {
    // Đọc ảnh đầu vào
    cv::Mat image = cv::imread(inputPath, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Không thể đọc ảnh: " << inputPath << std::endl;
        return;
    }

    // Tạo bản sao để xử lý
    cv::Mat original = image.clone();
    cv::Mat finalMask = cv::Mat::zeros(image.rows, image.cols, CV_8U);

    // BƯỚC 1: PHÂN TÍCH TỔNG QUAN ẢNH
    cv::Scalar meanColor = cv::mean(image);
    bool isDarkBg = (meanColor[0] + meanColor[1] + meanColor[2])/3 < 128;

    // BƯỚC 2: CHUẨN BỊ MẪU ĐỂ GIÚP GRABCUT
    // Tạo mask mẫu dựa vào các phương pháp đơn giản
    cv::Mat sampleMask = cv::Mat::zeros(image.rows, image.cols, CV_8U);

    // 2.1: Phát hiện cạnh để tìm biên của đối tượng
    cv::Mat edges;
    cv::cvtColor(image, edges, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(edges, edges, cv::Size(3, 3), 0);
    cv::Canny(edges, edges, 30, 100);

    // 2.2: Tìm countours từ biên
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Lọc các contour theo diện tích
    std::vector<std::vector<cv::Point>> largeContours;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > 100) { // ngưỡng diện tích
            largeContours.push_back(contour);
        }
    }

    // Vẽ các contour lớn vào mask mẫu
    cv::drawContours(sampleMask, largeContours, -1, cv::Scalar(255), -1);

    // 2.3: Phân tích màu sắc đặc trưng
    cv::Mat ycrcb;
    cv::cvtColor(image, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> ycrcbChannels;
    cv::split(ycrcb, ycrcbChannels);

    // Sử dụng K-means để phân cụm pixel
    cv::Mat data;
    ycrcbChannels[0].convertTo(data, CV_32F);
    data = data.reshape(1, data.total());

    cv::Mat labels, centers;
    const int K = 2; // Phân thành 2 cụm: foreground và background
    cv::kmeans(data, K, labels,
               cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 10, 1.0),
               3, cv::KMEANS_PP_CENTERS, centers);

    // Xác định cụm nào tương ứng với foreground (gần trung tâm ảnh)
    cv::Point imageCenter(image.cols/2, image.rows/2);
    cv::Mat colorMask = cv::Mat::zeros(image.rows, image.cols, CV_8U);

    int clusterIdx = 0;
    // Xác định cụm ít phổ biến hơn - thường là foreground
    int counts[K] = {0};
    for (int i = 0; i < labels.rows; i++) {
        counts[labels.at<int>(i)]++;
    }
    clusterIdx = (counts[0] < counts[1]) ? 0 : 1;

    // Tạo mask từ cụm foreground
    labels = labels.reshape(1, image.rows);
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            if (labels.at<int>(y, x) == clusterIdx) {
                colorMask.at<uchar>(y, x) = 255;
            }
        }
    }

    // BƯỚC 3: KẾT HỢP CÁC MASK MẪU
    cv::Mat combinedMask = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);

    // 3.1: Kết hợp từ các mask
    cv::bitwise_or(sampleMask, colorMask, combinedMask);

    // 3.2: Tạo Rectangle ROI lớn hơn để đảm bảo GrabCut không bỏ qua các vùng
    cv::Rect rect;
    if (!largeContours.empty()) {
        rect = cv::boundingRect(largeContours[0]);
        for (const auto& contour : largeContours) {
            rect |= cv::boundingRect(contour);
        }
        // Mở rộng hơn một chút
        int padding = std::min(50, std::min(rect.x, std::min(rect.y,
                                                             std::min(image.cols - rect.x - rect.width, image.rows - rect.y - rect.height))));
        rect -= cv::Point(padding, padding);
        rect += cv::Size(padding*2, padding*2);
    } else {
        // Nếu không có contour nào, sử dụng phần lớn của ảnh
        int margin = std::min(image.rows, image.cols) / 10;
        rect = cv::Rect(margin, margin, image.cols - 2*margin, image.rows - 2*margin);
    }

    // BƯỚC 4: TINH CHỈNH VỚI GRABCUT
    // Khởi tạo mask cho GrabCut
    cv::Mat grabMask = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
    cv::Mat bgModel, fgModel;

    // 4.1: Gán vùng các pixel trong mask mẫu là foreground có thể
    for (int y = 0; y < grabMask.rows; y++) {
        for (int x = 0; x < grabMask.cols; x++) {
            if (combinedMask.at<uchar>(y, x) > 0) {
                grabMask.at<uchar>(y, x) = cv::GC_PR_FGD;
            } else {
                grabMask.at<uchar>(y, x) = cv::GC_PR_BGD;
            }
        }
    }

    // 4.2: Nếu có vùng chắc chắn là foreground (từ contour), gán nó
    for (const auto& contour : largeContours) {
        cv::Point center = cv::Point(0, 0);
        for (const auto& point : contour) {
            center += point;
        }
        center.x /= contour.size();
        center.y /= contour.size();

        // Vùng xung quanh trung tâm contour là foreground xác định
        int radius = 5;
        cv::circle(grabMask, center, radius, cv::GC_FGD, -1);
    }

    // 4.3: Vùng ngoài rectangle là background chắc chắn
    for (int y = 0; y < grabMask.rows; y++) {
        for (int x = 0; x < grabMask.cols; x++) {
            if (x < rect.x || x >= rect.x + rect.width ||
                y < rect.y || y >= rect.y + rect.height) {
                grabMask.at<uchar>(y, x) = cv::GC_BGD;
            }
        }
    }

    // 4.4: Thực hiện GrabCut với mask đã chuẩn bị
    cv::grabCut(image, grabMask, rect, bgModel, fgModel, 5, cv::GC_INIT_WITH_MASK);

    // Tạo mask cuối cùng từ kết quả GrabCut
    cv::Mat grabCutMask1 = (grabMask == cv::GC_FGD);
    cv::Mat grabCutMask2 = (grabMask == cv::GC_PR_FGD);
    cv::Mat grabCutResult;
    cv::bitwise_or(grabCutMask1, grabCutMask2, grabCutResult);
    grabCutResult.convertTo(grabCutResult, CV_8U, 255);

    // BƯỚC 5: XỬ LÝ HẬU KỲ ĐỂ BẢO TOÀN CHI TIẾT TỐI
    // 5.1: Tính toán gradient ảnh để phát hiện biên chi tiết
    cv::Mat grayImg, gradient;
    cv::cvtColor(image, grayImg, cv::COLOR_BGR2GRAY);
    cv::Mat gradX, gradY;
    cv::Sobel(grayImg, gradX, CV_16S, 1, 0);
    cv::Sobel(grayImg, gradY, CV_16S, 0, 1);

    cv::convertScaleAbs(gradX, gradX);
    cv::convertScaleAbs(gradY, gradY);
    cv::addWeighted(gradX, 0.5, gradY, 0.5, 0, gradient);

    // 5.2: Tạo mask chi tiết dựa trên gradient
    cv::Mat detailMask;
    cv::threshold(gradient, detailMask, 50, 255, cv::THRESH_BINARY);

    // 5.3: Mở rộng mask chi tiết để kết nối các vùng
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::dilate(detailMask, detailMask, element);

    // 5.4: Chỉ giữ lại chi tiết gần foreground
    cv::Mat dilatedGrabCut;
    cv::dilate(grabCutResult, dilatedGrabCut, element, cv::Point(-1, -1), 3);
    cv::bitwise_and(detailMask, dilatedGrabCut, detailMask);

    // 5.5: Kết hợp với mask chính
    cv::bitwise_or(grabCutResult, detailMask, finalMask);

    // BƯỚC 6: LÀM MỊN KẾT QUẢ CUỐI CÙNG
    // Đảm bảo mask mịn và không bị nhiễu
    cv::medianBlur(finalMask, finalMask, 3);
    cv::morphologyEx(finalMask, finalMask, cv::MORPH_CLOSE,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

    // BƯỚC 7: TẠO ẢNH KẾT QUẢ VỚI ALPHA
    cv::Mat result;
    cv::cvtColor(original, result, cv::COLOR_BGR2BGRA);

    // Áp dụng mask vào kênh alpha
    cv::Mat resultChannels[4];  // Đổi tên biến để tránh xung đột
    cv::split(result, resultChannels);
    resultChannels[3] = finalMask;
    cv::merge(resultChannels, 4, result);

    // Lưu ảnh kết quả
    if (!cv::imwrite(outputPath, result)) {
        std::cerr << "Lỗi khi lưu ảnh: " << outputPath << std::endl;
    } else {
        std::cout << "Xóa background thành công: " << outputPath << std::endl;
    }
}
}