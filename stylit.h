#ifndef STYLIT_H
#define STYLIT_H

#include "Eigen/Dense"
using namespace Eigen;

#include "rgba.h"

struct Patch {
    // vectors of length 75 for now
    VectorXf buffer;
    Vector2i coordinates;
    std::vector<Patch*> neighbor_patches;
};

struct Image {
    int width;
    int height;
    std::vector<Patch*> patches_orignal;
    std::vector<Patch*> patches_LPE1;
    std::vector<Patch*> patches_LPE2;
    std::vector<Patch*> patches_LPE3;
    void init_patches(const std::vector<RGBA> &original,const std::vector<RGBA> &LPE1,
                     const std::vector<RGBA> &LPE2, const std::vector<RGBA> &LPE3);
};



class Stylit
{
public:

    Stylit();

    Image source_image;
    Image target_image;

    std::unordered_map<int, Vector2i> NNF;

    std::vector<RGBA> output;
};

#endif // STYLIT_H
