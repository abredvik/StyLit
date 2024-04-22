#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "util.h"

class Patchmatcher
{
public:
    Patchmatcher(int width, int height);

    std::vector<std::pair<int, double>> errors;

    void random_search(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    void randomize_NNF(NNF_t& NNF, int imgSize, int width, int height);

    void propagate_odd(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    void propagate_even(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy);

    NNF_t patch_match(const Image& src, const Image& tgt);

    double mu = 2;
};

double Energy(const std::vector<VectorXf*>& A, const std::vector<VectorXf*>& B,
              const VectorXf& Aprime, const VectorXf& Bprime, double mu);

double Distance(const VectorXf& A, const VectorXf& B);

std::vector<Patch*> get_patches(const std::vector<RGBA>& img, int width, int height);

std::vector<RGBA> recreate_image(NNF_t NNF, const Image& target);

#endif // PATCHMATCH_H
