#ifndef CONVOLVE_H
#define CONVOLVE_H

#include <iostream>
#include <numbers>

#include "Eigen/Dense"
#include "util.h"

using namespace Eigen;


class Convolve
{
public:
   // Convolve();
    //void handle_blur(std::vector<Vector3f> &image, int width);
    std::vector<Vector3f> convolve(std::vector<Vector3f>& img, int width, const std::vector<float>& kernel, bool x_dir);
    float gaussian_func(float x);
    std::vector<float> gaussian_kernel(int radius);
    auto& getPixelRepeated(auto& data, int width, int height, int x, int y);
    std::vector<RGBA> sharpen(std::vector<RGBA> stylized_image_RGBA, int width, int height, std::vector<float> blur_kernel);
};

#endif // CONVOLVE_H
