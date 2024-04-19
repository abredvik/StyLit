#ifndef PATCHMATCH_H
#define PATCHMATCH_H

#include "util.h"

//class Patchmatcher
//{
//public:
//    Patchmatcher();

//    std::vector<std::pair<int, float>> errors;

//    NNF_t patch_match();
//};

std::vector<VectorXf> get_patches(const std::vector<RGBA>& img, int width, int height);

double Distance(VectorXf A, VectorXf B);

void random_search(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width);

void randomize_NNF(NNF_t& NNF, int imgSize, int width, int height);

void propagate_odd(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width, int height);

void propagate_even(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width, int height);

NNF_t patch_match(const std::vector<VectorXf>& src_patches, const std::vector<VectorXf>& tgt_patches, int width, int height);

std::vector<RGBA> recreate_image(NNF_t NNF, const std::vector<RGBA>& target, int width);


#endif // PATCHMATCH_H
