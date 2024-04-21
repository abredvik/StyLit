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

    Stylit(int source_width, int source_height, int target_width, int target_height,
           std::vector<RGBA> source_color_rbgs, std::vector<RGBA> source_LPE1_rbgs, std::vector<RGBA> source_LPE2_rbgs, std::vector<RGBA> source_LPE3_rbgs,
           std::vector<RGBA> target_color_rbgs, std::vector<RGBA> target_LPE1_rbgs, std::vector<RGBA> target_LPE2_rbgs, std::vector<RGBA> target_LPE3_rbgs,
           std::vector<RGBA> source_style_rbgs);

    Image source_image;
    Image target_image;

    NNF_t final_NNF;

    std::vector<RGBA> output;

    void stylit_algorithm();

    std::pair<int, float> calculate_error_budget();

    void fill_gaps();

    void average(int index);

    std::vector<RGBA> run(int iterations);
};

#endif // STYLIT_H
