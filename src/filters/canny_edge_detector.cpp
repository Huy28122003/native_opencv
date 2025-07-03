#include "../native_opencv.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern "C"
{
    FUNCTION_ATTRIBUTE
    void cannyEdgeDetector(char *path, char *output, float threshold, float ratio)
    {

        // Read an image from file
        cv::Mat image = cv::imread(path, cv::IMREAD_UNCHANGED);

        if (image.empty())
        {
            std::cerr << "Error loading image." << std::endl;
            return;
        }

        // Convert the image to grayscale if it's not already
        cv::Mat grayImage;
        if (image.channels() > 1)
        {
            cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        }
        else
        {
            grayImage = image.clone();
        }

        // Apply GaussianBlur to reduce noise and improve edge detection
        cv::GaussianBlur(grayImage, grayImage, cv::Size(5, 5), 1.5, 1.5);

        // Apply Canny edge detector
        cv::Mat edges;
        cv::Canny(grayImage, edges, threshold, threshold * ratio);

        // Create a new image with a transparent background
        cv::Mat result(image.rows, image.cols, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        // Set the alpha channel to represent the edges
        for (int i = 0; i < result.rows; ++i)
        {
            for (int j = 0; j < result.cols; ++j)
            {
                if (edges.at<uchar>(i, j) > 0)
                {
                    result.at<cv::Vec4b>(i, j) = cv::Vec4b(0, 0, 0, 255); // Set alpha to 255 for edges
                }
                else
                {
                    result.at<cv::Vec4b>(i, j) = cv::Vec4b(0, 0, 0, 0); // Set alpha to 0 for background
                }
            }
        }

        // Save the result to a new image file (optional)
        cv::imwrite(output, result);
    }
}
