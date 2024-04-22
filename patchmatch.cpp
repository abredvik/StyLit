#include "patchmatch.h"

#include <random>

Patchmatcher::Patchmatcher()
{

}

//struct Patch {
//    // vectors of length 75 for now
//    VectorXf buffer;
//    Vector2i coordinates;
//    std::vector<int> neighbor_patches;
//    bool is_matched;
//};

std::vector<Patch*> get_patches(const std::vector<RGBA>& img, int width, int height) {
    // return a bunch of 5 by 5 (or representation of) patches
    // one for each pixel
    std::vector<Patch*> result;
    result.reserve(img.size());
    int window_width = 5;
    int w = window_width / 2;

    for (int p = 0; p < img.size(); ++p) {
        Patch *patch = new Patch();
        patch->buffer = VectorXf(3 * window_width * window_width);
        patch->coordinates = index_to_position(p, width);
        patch->neighbor_patches.reserve(window_width * window_width);
        patch->is_matched = false;

        const Vector2i& xy = patch->coordinates;
        int row = xy[1], col = xy[0];

        for (int i = row - w; i < row + w; ++i) {
            for (int j = col - w; j < col + w; ++j) {
                int patch_index = 3 * pos_to_index(Vector2i(j - col + w, i - row + w), window_width);
                if (i < 0 || i >= height || j < 0 || j >= width) {
                    const RGBA& center = img[pos_to_index(Vector2i(col, row), width)];
                    patch->buffer[patch_index] = uint8_to_float(center.r);
                    patch->buffer[patch_index + 1] = uint8_to_float(center.g);
                    patch->buffer[patch_index + 2] = uint8_to_float(center.b);
                } else {
                    int pixel_index = pos_to_index(Vector2i(j, i), width);
                    const RGBA& rgb = img[pixel_index];
                    patch->buffer[patch_index] = uint8_to_float(rgb.r);
                    patch->buffer[patch_index + 1] = uint8_to_float(rgb.g);
                    patch->buffer[patch_index + 2] = uint8_to_float(rgb.b);
                    patch->neighbor_patches.push_back(pixel_index);
                }
            }
        }

        result.push_back(patch);
    }

    return result;

}

double Distance(const VectorXf& A, const VectorXf& B) {
//    return (|| A'(p) - B'(q) ||)^2 + mu (|| A(p) - B(q) ||)^2
    return (B - A).norm();
}

double Energy(const std::vector<VectorXf*>& A, const std::vector<VectorXf*>& B,
              const VectorXf& Aprime, const VectorXf& Bprime, double mu) {
    double result = 0.0;
    for (int i = 0; i < 4; ++i) {
        result += Distance(Aprime, Bprime) + mu * Distance(*A[i], *B[i]);
    }
    return result;
}

std::random_device                      rand_dev;
std::mt19937                            generator(rand_dev());
std::uniform_real_distribution<double>   distr(-1.0, 1.0);

void Patchmatcher::random_search(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy){
    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_orignal[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i v0 = NNF[pos_to_index(xy, src.width)]; // this is the offset for the patch of interest
    std::vector<VectorXf*> final_patches(4);
    int index = pos_to_index(xy + v0, tgt.width);
    const VectorXf& final_stylized_patch = tgt.patches_stylized[index]->buffer;
    final_patches[0] = &(tgt.patches_orignal[index]->buffer);
    final_patches[1] = &(tgt.patches_LPE1[index]->buffer);
    final_patches[2] = &(tgt.patches_LPE2[index]->buffer);
    final_patches[3] = &(tgt.patches_LPE3[index]->buffer);

    double final_energy = Energy(src_patches, final_patches, src_stylized_patch, final_stylized_patch, mu);

    double alpha = 0.5;
    int i = 0;
    int w = src.width;

    while (w * std::pow(alpha, i) > 1) {
        Vector2d R(distr(generator), distr(generator));
        Vector2i ui = (v0.cast<double>() + (w * std::pow(alpha, i) * R)).cast<int>();
        Vector2i newCoord = xy + ui;
        newCoord = newCoord.cwiseMax(0).cwiseMin(src.width - 1);
        std::vector<VectorXf*> candidate_patches(4);
        int candidate_index = pos_to_index(newCoord, src.width);
        const VectorXf& candidate_stylized_patch = tgt.patches_stylized[candidate_index]->buffer;
        candidate_patches[0] = &(tgt.patches_orignal[candidate_index]->buffer);
        candidate_patches[1] = &(tgt.patches_LPE1[candidate_index]->buffer);
        candidate_patches[2] = &(tgt.patches_LPE2[candidate_index]->buffer);
        candidate_patches[3] = &(tgt.patches_LPE3[candidate_index]->buffer);
        double candidate_energy = Energy(src_patches, candidate_patches, src_stylized_patch, candidate_stylized_patch, mu);

        if (candidate_energy < final_energy){
            v0 = newCoord - xy;
            final_patches = candidate_patches;
            final_energy = candidate_energy;
        }

        ++i;
    }

    NNF[pos_to_index(xy, src.width)] = v0;
}

void Patchmatcher::randomize_NNF(NNF_t& NNF, int imgSize, int width, int height) {
    for (int i = 0; i < imgSize; ++i) {
        Vector2i xy = index_to_position(i, width);
        Vector2i newXY(rand() % width, rand() % height);
        NNF[i] = (newXY - xy);
    }
}

void Patchmatcher::propagate_odd(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool left_valid = (xy[0] > 0);
    bool top_valid = (xy[1] > 0);

    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_orignal[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    std::vector<VectorXf*> tgt_center_patches(4);
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;
    tgt_center_patches[0] = &(tgt.patches_orignal[tgt_center_index]->buffer);
    tgt_center_patches[1] = &(tgt.patches_LPE1[tgt_center_index]->buffer);
    tgt_center_patches[2] = &(tgt.patches_LPE2[tgt_center_index]->buffer);
    tgt_center_patches[3] = &(tgt.patches_LPE3[tgt_center_index]->buffer);


    Vector2i tgt_left;
    Vector2i tmp;
    std::vector<VectorXf*> tgt_left_patches(4);
    VectorXf tgt_left_stylized_patch;
    if (left_valid) {
        tgt_left = NNF[pos_to_index(xy - Vector2i(1, 0), tgt.width)];
        tmp = xy + tgt_left;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_left_index = pos_to_index(tmp, tgt.width);
            tgt_left_stylized_patch = tgt.patches_stylized[tgt_left_index]->buffer;
            tgt_left_patches[0] = &(tgt.patches_orignal[tgt_left_index]->buffer);
            tgt_left_patches[1] = &(tgt.patches_LPE1[tgt_left_index]->buffer);
            tgt_left_patches[2] = &(tgt.patches_LPE2[tgt_left_index]->buffer);
            tgt_left_patches[3] = &(tgt.patches_LPE3[tgt_left_index]->buffer);
        } else {
            left_valid = false;
        }
    }

    Vector2i tgt_top;
    std::vector<VectorXf*> tgt_top_patches(4);
    VectorXf tgt_top_stylized_patch;
    if (top_valid) {
        tgt_top = NNF[pos_to_index(xy - Vector2i(0, 1), tgt.width)];
        tmp = xy + tgt_top;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_top_index = pos_to_index(tmp, tgt.width);
            tgt_top_stylized_patch = tgt.patches_stylized[tgt_top_index]->buffer;
            tgt_top_patches[0] = &(tgt.patches_orignal[tgt_top_index]->buffer);
            tgt_top_patches[1] = &(tgt.patches_LPE1[tgt_top_index]->buffer);
            tgt_top_patches[2] = &(tgt.patches_LPE2[tgt_top_index]->buffer);
            tgt_top_patches[3] = &(tgt.patches_LPE3[tgt_top_index]->buffer);
        } else {
            top_valid = false;
        }
    }

    double current_dist = Energy(src_patches, tgt_center_patches, src_stylized_patch, tgt_center_stylized_patch, mu);//Distance(src_patches, tgt_center_patches);
    int index = pos_to_index(xy, src.width);
    if (left_valid && (Energy(src_patches, tgt_left_patches, src_stylized_patch, tgt_left_stylized_patch, mu) < current_dist)) {
        NNF[index] = tgt_left;
    }

    if (top_valid && (Energy(src_patches, tgt_top_patches, src_stylized_patch, tgt_top_stylized_patch, mu) < current_dist)) {
        NNF[index] = tgt_top;
    }
}

void Patchmatcher::propagate_even(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool right_valid = (xy[0] < src.width - 1);
    bool bottom_valid = (xy[1] < src.height - 1);

    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_orignal[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    std::vector<VectorXf*> tgt_center_patches(4);
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;
    tgt_center_patches[0] = &(tgt.patches_orignal[tgt_center_index]->buffer);
    tgt_center_patches[1] = &(tgt.patches_LPE1[tgt_center_index]->buffer);
    tgt_center_patches[2] = &(tgt.patches_LPE2[tgt_center_index]->buffer);
    tgt_center_patches[3] = &(tgt.patches_LPE3[tgt_center_index]->buffer);


    Vector2i tgt_right;
    Vector2i tmp;
    std::vector<VectorXf*> tgt_right_patches(4);
    VectorXf tgt_right_stylized_patch;
    if (right_valid) {
        tgt_right = NNF[pos_to_index(xy + Vector2i(1, 0), tgt.width)];
        tmp = xy + tgt_right;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_right_index = pos_to_index(tmp, tgt.width);
            tgt_right_stylized_patch = tgt.patches_stylized[tgt_right_index]->buffer;
            tgt_right_patches[0] = &(tgt.patches_orignal[tgt_right_index]->buffer);
            tgt_right_patches[1] = &(tgt.patches_LPE1[tgt_right_index]->buffer);
            tgt_right_patches[2] = &(tgt.patches_LPE2[tgt_right_index]->buffer);
            tgt_right_patches[3] = &(tgt.patches_LPE3[tgt_right_index]->buffer);
        } else {
            right_valid = false;
        }
    }

    Vector2i tgt_bottom;
    std::vector<VectorXf*> tgt_bottom_patches(4);
    VectorXf tgt_bottom_stylized_patch;
    if (bottom_valid) {
        tgt_bottom = NNF[pos_to_index(xy + Vector2i(0, 1), tgt.width)];
        tmp = xy + tgt_bottom;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_bottom_index = pos_to_index(tmp, tgt.width);
            tgt_bottom_stylized_patch = tgt.patches_stylized[tgt_bottom_index]->buffer;
            tgt_bottom_patches[0] = &(tgt.patches_orignal[tgt_bottom_index]->buffer);
            tgt_bottom_patches[1] = &(tgt.patches_LPE1[tgt_bottom_index]->buffer);
            tgt_bottom_patches[2] = &(tgt.patches_LPE2[tgt_bottom_index]->buffer);
            tgt_bottom_patches[3] = &(tgt.patches_LPE3[tgt_bottom_index]->buffer);
        } else {
            bottom_valid = false;
        }
    }

    double current_dist = Energy(src_patches, tgt_center_patches, src_stylized_patch, tgt_center_stylized_patch, mu);
    int index = pos_to_index(xy, src.width);
    if (right_valid && (Energy(src_patches, tgt_right_patches, src_stylized_patch, tgt_right_stylized_patch, mu) < current_dist)) {
        NNF[index] = tgt_right;
    }

    if (bottom_valid && (Energy(src_patches, tgt_bottom_patches, src_stylized_patch, tgt_bottom_stylized_patch, mu) < current_dist)) {
        NNF[index] = tgt_bottom;
    }
}

NNF_t Patchmatcher::patch_match(const Image& src, const Image& tgt) {
    // fill NNF randomly
    NNF_t NNF;
    int imgSize = src.patches_orignal.size();
    int numIterations = 6;

    randomize_NNF(NNF, imgSize, src.width, src.height);

    for (int iteration = 1; iteration <= numIterations; ++iteration) {
        if (iteration % 2) {
            // scanline order
            for (int j = 0; j < imgSize; ++j) {
                const Vector2i& xy = src.patches_orignal[j]->coordinates;

                // propagation
                propagate_odd(NNF, src, tgt, xy);

                // random search
                random_search(NNF, src, tgt, xy);
            }

        } else {
            // reverse scanline order
            for (int j = imgSize - 1; j >= 0; --j) {
                Vector2i xy = index_to_position(j, src.width);

                // propagation
                propagate_even(NNF, src, tgt, xy);

                // random search
                random_search(NNF, src, tgt, xy);
            }

        }
    }

    return NNF;
}

std::vector<RGBA> Patchmatcher::recreate_image(NNF_t NNF, const std::vector<RGBA>& target, int width) {
    std::vector<RGBA> reconstruction(target.size());
    for (int i = 0; i < target.size(); ++i) {
        reconstruction[i] = target[pos_to_index(index_to_position(i, width) + NNF[i], width)];
    }
    return reconstruction;
}

