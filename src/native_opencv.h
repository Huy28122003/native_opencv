#ifdef __cplusplus

#include "common.h"

extern "C"
{
#endif

const char *opencvVersion();

void gausianBlur(char *path);

void cannyEdgeDetector(char *path, char *output, float threshold, float ratio);

void removeWhiteBg(char *path, char *output, int threshold);

void sketch(char *path, char *output);

void removeBg(char *path, char *output);

void rough(char *path, char *output);

void sobelEdgeDetector(char *path);

void cannyEdgeDetectorV2(
        char *inputPath,
        char *outputPath,
        int gaussianKernelSize,
        float gaussianSigma,
        float cannyLowThresh,
        float cannyHighThresh,
        int edgeR,
        int edgeG,
        int edgeB,
        int edgeA,
        int dilationSize
);


#ifdef __cplusplus
}
#endif
