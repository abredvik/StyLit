#ifndef GAUSSIANPYRAMID_H
#define GAUSSIANPYRAMID_H

#include "rgba.h"
#include "scale.h"

typedef struct Pyramid{
    std::vector<std::vector<RGBA>> color;
    std::vector<std::vector<RGBA>> LPE1;
    std::vector<std::vector<RGBA>> LPE2;
    std::vector<std::vector<RGBA>> LPE3;
    std::vector<std::vector<RGBA>> LPE4;
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
