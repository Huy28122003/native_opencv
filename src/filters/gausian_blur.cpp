#include "../native_opencv.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

extern "C"
{
    FUNCTION_ATTRIBUTE
    void gausianBlur(char *path)
    {
        Mat img = imread(path);
        Mat temp;
        GaussianBlur(img, temp, Size(75, 75), 0, 0);
        imwrite(path, temp);
    }
}
