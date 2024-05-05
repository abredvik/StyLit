#ifndef SCALE_H
#define SCALE_H

#include <iostream>

#include "rgba.h"

#include "Eigen/Dense"
using namespace Eigen;


class Scale
{
public:
    //scale();
    double triangle_filter(double x, double radius, float scale);
    RGBA back_map(const std::vector<RGBA> &src, int width, int x, int y, float scale, bool x_dir) ;
    std::vector<RGBA> handle_scale(const std::vector<RGBA> &image, int width, int height, float scaleX, float scaleY);
    auto& getPixelRepeated(auto& data, int width, int height, int x, int y);
};

#endif // SCALE_H
