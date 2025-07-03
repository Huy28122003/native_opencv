#include "../native_opencv.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

extern "C"
{
    FUNCTION_ATTRIBUTE
    void sobelEdgeDetector(char *path)
    {
        Mat src, src_gray;

        src = imread(path);
        cvtColor(src, src_gray, COLOR_BGR2GRAY);

        Mat img_blur;
        GaussianBlur(src_gray, img_blur, Size(3, 3), 0);

        Mat dst;
        Sobel(img_blur, dst, CV_64F, 1, 1, 5);
        // dst = Scalar::all(0);
        // src.copyTo(dst, detected_edges);

        imwrite(path, dst);
    }
}
