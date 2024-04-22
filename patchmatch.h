#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "util.h"

class Patchmatcher
{
public:
    Patchmatcher();

    std::vector<std::pair<int, float>> errors;

    void random_search(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    void randomize_NNF(NNF_t& NNF, int imgSize, int width, int height);

    void propagate_odd(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    void propagate_even(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    NNF_t patch_match(const Image& src, const Image& tgt);

    std::vector<RGBA> recreate_image(NNF_t NNF, const std::vector<RGBA>& target, int width);

    double mu = 2;
};

double Distance(const VectorXf& A, const VectorXf& B);

std::vector<Patch*> get_patches(const std::vector<RGBA>& img, int width, int height);

#endif // PATCHMATCH_H
