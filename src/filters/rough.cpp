#include "../native_opencv.h"
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C"
{
FUNCTION_ATTRIBUTE
void rough(char *path, char *output)
{
    // Đọc ảnh và kiểm tra
    cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
    if (image.empty())
    {
        std::cerr << "Không thể đọc ảnh, kiểm tra lại đường dẫn: " << path << std::endl;
        return;
    }

    // XÓA NỀN ẢNH BẰNG GrabCut
    cv::Mat mask(image.size(), CV_8UC1, cv::GC_PR_BGD); // Đánh dấu toàn bộ là nền trước
    cv::Rect rect(10, 10, image.cols - 20, image.rows - 20); // Giả định vật thể nằm ở trung tâm
    cv::grabCut(image, mask, rect, cv::Mat(), cv::Mat(), 5, cv::GC_INIT_WITH_RECT);

    // Chuyển các vùng tiền cảnh thành trắng, nền thành đen
    cv::Mat foregroundMask;
    cv::compare(mask, cv::GC_FGD, foregroundMask, cv::CMP_EQ);

    // Tạo ảnh không nền
    cv::Mat no_bg;
    image.copyTo(no_bg, foregroundMask);

    // CHUYỂN ẢNH SANG HSV ĐỂ LỌC DA
    cv::Mat hsv;
    cv::cvtColor(no_bg, hsv, cv::COLOR_BGR2HSV);

    // Ngưỡng màu da (có thể cần điều chỉnh theo ánh sáng)
    cv::Scalar lower_skin(0, 40, 60);
    cv::Scalar upper_skin(25, 255, 255);

    // Tạo mask cho vùng da
    cv::Mat skin_mask;
    cv::inRange(hsv, lower_skin, upper_skin, skin_mask);

    // Làm mịn mask da để loại bỏ nhiễu
    cv::GaussianBlur(skin_mask, skin_mask, cv::Size(5, 5), 0);

    // Phát hiện biên cạnh vùng da
    cv::Mat edges;
    cv::Canny(skin_mask, edges, 100, 200);

    // Tìm đường viền
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty())
    {
        std::cerr << "Không tìm thấy vùng da nào trong ảnh." << std::endl;
        return;
    }

    // Tìm đường viền lớn nhất theo diện tích
    size_t largest_contour_index = 0;
    double max_area = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area > max_area)
        {
            max_area = area;
            largest_contour_index = i;
        }
    }

    std::vector<cv::Point> largest_contour = contours[largest_contour_index];

    // Tìm Convex Hull (bao lồi)
    std::vector<int> hull;
    cv::convexHull(largest_contour, hull, false, false);

    // Tìm các vùng lồi lõm (Convexity Defects)
    std::vector<cv::Vec4i> convexity_defects;
    cv::convexityDefects(largest_contour, hull, convexity_defects);

    // Vẽ kết quả lên ảnh gốc không nền
    cv::Mat result = no_bg.clone();
    cv::drawContours(result, contours, largest_contour_index, cv::Scalar(0, 255, 0), 2); // Xanh lá
    std::vector<cv::Point> hull_points;
    for (int i : hull)
    {
        hull_points.push_back(largest_contour[i]);
    }
    cv::polylines(result, hull_points, true, cv::Scalar(255, 0, 0), 2); // Xanh dương

    // Đánh dấu các điểm lõm
    for (const auto &defect : convexity_defects)
    {
        cv::Point start = largest_contour[defect[0]];
        cv::Point end = largest_contour[defect[1]];
        cv::Point depth_point = largest_contour[defect[2]];
        float depth = defect[3] / 256.0;

        // Vẽ điểm lõm (màu đỏ)
        cv::circle(result, depth_point, 5, cv::Scalar(0, 0, 255), -1);

        // Vẽ đường nối vùng lõm
        cv::line(result, start, depth_point, cv::Scalar(255, 255, 0), 2);
        cv::line(result, end, depth_point, cv::Scalar(255, 255, 0), 2);
    }

    // Lưu ảnh kết quả
    if (!cv::imwrite(output, result))
    {
        std::cerr << "Lỗi khi lưu ảnh: " << output << std::endl;
    }
    else
    {
        std::cout << "Ảnh phân tích lồi lõm trên da (đã xóa nền) đã được lưu: " << output << std::endl;
    }
}
}
