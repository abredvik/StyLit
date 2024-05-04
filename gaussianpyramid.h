#ifndef GAUSSIANPYRAMID_H
#define GAUSSIANPYRAMID_H

#include "rgba.h"
#include "scale.h"
#include <vector>

typedef struct Pyramid{
    std::vector<std::vector<std::vector<RGBA>>> LPEs;
    std::vector<std::vector<RGBA>> style;
} Pyramid_t;

class Gaussianpyramid
{
public:
    std::vector<std::vector<RGBA>> create_pyrimid(int num_iterations, std::vector<RGBA> image, int width, int height);
private:
    Scale scale;
};

#endif // GAUSSIANPYRAMID_H
