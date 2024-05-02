#ifndef STYLIT_H
#define STYLIT_H

#include "Eigen/Dense"
using namespace Eigen;

#include "rgba.h"
#include "util.h"

class Stylit
{
public:

    Stylit();

    NNF_t final_NNF;

    NNF_t final_reverse_NNF;

    void stylit_algorithm(const Image& src, Image& tgt, int current_iteration);

    std::pair<int, double> calculate_k_and_error_budget(std::vector<std::pair<int, double>> &errors);

    int calculate_k(std::vector<std::pair<int, double>> &errors);

    void resolve_unmatched(const Image& src, Image& tgt, const std::unordered_set<int>& unmatched);

    RGBA average(int index, const Image& src, Image& tgt);

    std::vector<RGBA> run(Image& src, Image& tgt, int iterations);

    Vector2i nearest_neighbor(const Image& src, const VectorXf& tgt_patch, const VectorXf& tgt_style, Vector2i xy);

private:
    int window_width = 5;
    double mu = 2.0;
};

void init_image(const std::vector<QString>& LPEnames, const QString styleName, Image& img, int num_iterations);

#endif // STYLIT_H
