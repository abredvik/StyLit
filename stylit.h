#ifndef STYLIT_H
#define STYLIT_H

#include "Eigen/Dense"
using namespace Eigen;

#include "rgba.h"
#include "util.h"
#include "patchmatch.h"

class Stylit
{
public:

    Stylit();

    NNF_t final_NNF;

    std::vector<RGBA> output;

    void stylit_algorithm(const Image& src, Image& tgt);

    int calculate_error_budget(std::vector<std::pair<int, double>> &errors);

    void resolve_unmatched(const Image& src, Image& tgt);

    void average(int index);

    std::vector<RGBA> run(const Image& src, Image& tgt, int iterations);

    Vector2i nearest_neighbor(const Image& src, std::vector<VectorXf*> tgt_patches, const VectorXf& tgt_style, Vector2i xy);
};

void init_image(const std::vector<QString>& filepath, Image& img);

#endif // STYLIT_H
