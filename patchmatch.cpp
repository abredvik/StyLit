#include "patchmatch.h"

#include <random>
#include <unordered_set>

Patchmatcher::Patchmatcher(int width, int height) : errors(width * height) {}

//struct Patch {
//    // vectors of length 75 for now
//    VectorXf buffer;
//    Vector2i coordinates;
//    std::vector<int> neighbor_patches;
//    bool is_matched;
//};

std::pair<std::vector<Patch*>, std::vector<Patch*>> get_patches(const std::vector<std::vector<RGBA>>& LPEs,
                                                                const std::vector<RGBA>& style,
                                                                int window_width, int width, int height) {
    // return a bunch of 5 by 5 (or representation of) patches

    // one for each pixel
    const std::vector<RGBA>& img = LPEs[0];
    std::vector<Patch*> LPEresult(img.size());
    std::vector<Patch*> styleResult(img.size());
    int w = window_width / 2;

    int LPE_buffer_len = 3 * window_width * window_width;

    for (int p = 0; p < img.size(); ++p) {
        LPEresult[p] = new Patch();
        LPEresult[p]->buffer = VectorXf(LPE_buffer_len * LPEs.size());
        styleResult[p] = new Patch();
        styleResult[p]->buffer = VectorXf(LPE_buffer_len);
    }

#pragma omp parallel for
    for (int p = 0; p < img.size(); ++p) {
        Patch *LPEpatch = LPEresult[p];
        LPEpatch->coordinates = index_to_position(p, width);
        LPEpatch->is_matched = false;
        Patch *stylePatch = styleResult[p];
        stylePatch->coordinates = index_to_position(p, width);
        stylePatch->is_matched = false;

        const Vector2i& xy = LPEpatch->coordinates;
        int row = xy[1], col = xy[0];

        for (int i = row - w; i <= row + w; ++i) {
            for (int j = col - w; j <= col + w; ++j) {
                for (int k = 0; k < LPEs.size(); ++k) {
                    int patch_index = (k * LPE_buffer_len) + 3 * pos_to_index(Vector2i(j - col + w, i - row + w), window_width);
                    if (i < 0 || i >= height || j < 0 || j >= width) {
                        const RGBA& center = LPEs[k][pos_to_index(Vector2i(col, row), width)];
                        LPEpatch->buffer[patch_index] = uint8_to_float(center.r);
                        LPEpatch->buffer[patch_index + 1] = uint8_to_float(center.g);
                        LPEpatch->buffer[patch_index + 2] = uint8_to_float(center.b);
                        if (k == 0) {
                            LPEpatch->neighbor_patches.push_back(-1);
                            const RGBA& styleCenter = style[pos_to_index(Vector2i(col, row), width)];
                            stylePatch->buffer[patch_index] = uint8_to_float(styleCenter.r);
                            stylePatch->buffer[patch_index + 1] = uint8_to_float(styleCenter.g);
                            stylePatch->buffer[patch_index + 2] = uint8_to_float(styleCenter.b);
                            stylePatch->neighbor_patches.push_back(-1);
                        }
                    } else {
                        int pixel_index = pos_to_index(Vector2i(j, i), width);
                        const RGBA& rgb = LPEs[k][pixel_index];
                        LPEpatch->buffer[patch_index] = uint8_to_float(rgb.r);
                        LPEpatch->buffer[patch_index + 1] = uint8_to_float(rgb.g);
                        LPEpatch->buffer[patch_index + 2] = uint8_to_float(rgb.b);
                        if (k == 0) {
                            LPEpatch->neighbor_patches.push_back(pixel_index);
                            const RGBA& stylergb = style[pixel_index];
                            stylePatch->buffer[patch_index] = uint8_to_float(stylergb.r);
                            stylePatch->buffer[patch_index + 1] = uint8_to_float(stylergb.g);
                            stylePatch->buffer[patch_index + 2] = uint8_to_float(stylergb.b);
                            stylePatch->neighbor_patches.push_back(pixel_index);
                        }
                    }
                }
            }
        }
    }

    return std::make_pair(LPEresult, styleResult);

}

inline double Distance(const VectorXf& A, const VectorXf& B) {
    return (B - A).squaredNorm();
}

double Energy(const VectorXf& A, const VectorXf& B,
              const VectorXf& Aprime, const VectorXf& Bprime, double mu) {
//    return (|| A'(p) - B'(q) ||)^2 + mu (|| A(p) - B(q) ||)^2
    return Distance(Aprime, Bprime) + mu * Distance(A, B);
}

std::random_device                      rand_dev;
std::mt19937                            generator(rand_dev());
std::uniform_real_distribution<double>   distr(-1.0, 1.0);

void Patchmatcher::random_search(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy){
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_patch = src.patches_original[src_index]->buffer;
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;

    Vector2i v0 = NNF[pos_to_index(xy, src.width)]; // this is the offset for the patch of interest

    int index = pos_to_index(xy + v0, tgt.width);
    VectorXf* final_patch;
    final_patch = &(tgt.patches_original[index]->buffer);
    const VectorXf& final_stylized_patch = tgt.patches_stylized[index]->buffer;

    double final_energy = Energy(src_patch, *final_patch, src_stylized_patch, final_stylized_patch, this->mu);

    double alpha = 0.5;
    int i = 0;
    int w = src.width;

    while (w * std::pow(alpha, i) > 1) {
        Vector2d R(distr(generator), distr(generator));
        Vector2i ui = (v0.cast<double>() + (w * std::pow(alpha, i) * R)).cast<int>();
        Vector2i newCoord = xy + ui;
        newCoord = newCoord.cwiseMax(0).cwiseMin(tgt.width - 1);
        int candidate_index = pos_to_index(newCoord, tgt.width);
        VectorXf* candidate_patch = &(tgt.patches_original[candidate_index]->buffer);
        const VectorXf& candidate_stylized_patch = tgt.patches_stylized[candidate_index]->buffer;
        double candidate_energy = Energy(src_patch, *candidate_patch, src_stylized_patch, candidate_stylized_patch, this->mu);

        if (candidate_energy < final_energy && !(tgt.patches_original[candidate_index]->is_matched)){
            v0 = newCoord - xy;
            final_patch = candidate_patch;
            final_energy = candidate_energy;
        }

        ++i;
    }

    NNF[src_index] = v0;
    errors[src_index] = std::make_pair(src_index, final_energy);
}

int random_pick(const std::unordered_set<int>& unmatched) {
    auto it = unmatched.begin();
    int n = rand() % unmatched.size();
    std::advance(it, n);
    return *it;
}

void Patchmatcher::randomize_NNF(NNF_t& NNF, const Image& src, const Image& tgt, std::unordered_set<int>& unmatched) {
    for (int i = 0; i < src.patches_original.size(); ++i) {
        Vector2i xy = index_to_position(i, src.width);
        Vector2i newXY = index_to_position(random_pick(unmatched), tgt.width);
        NNF[i] = (newXY - xy);

        int src_index = i;
        const VectorXf& src_patch = src.patches_original[src_index]->buffer;
        const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;

        int tgt_index = pos_to_index(newXY, tgt.width);
        const VectorXf& tgt_patch = tgt.patches_original[tgt_index]->buffer;
        const VectorXf& tgt_stylized_patch = tgt.patches_stylized[tgt_index]->buffer;

        errors[i] = std::make_pair(i, Energy(src_patch, tgt_patch, src_stylized_patch, tgt_stylized_patch, this->mu));
    }
}

void Patchmatcher::propagate_odd(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool left_valid = (xy[0] > 0);
    bool top_valid = (xy[1] > 0);

    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_patch = src.patches_original[src_index]->buffer;
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_patch = tgt.patches_original[tgt_center_index]->buffer;
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;

    Vector2i tgt_left;
    Vector2i tmp;
    VectorXf* tgt_left_patch;
    VectorXf* tgt_left_stylized_patch;
    if (left_valid) {
        tgt_left = NNF[pos_to_index(xy - Vector2i(1, 0), tgt.width)];
        tmp = xy + tgt_left;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_left_index = pos_to_index(tmp, tgt.width);
            tgt_left_patch = &(tgt.patches_original[tgt_left_index]->buffer);
            tgt_left_stylized_patch = &(tgt.patches_stylized[tgt_left_index]->buffer);
            if (tgt.patches_original[tgt_left_index]->is_matched) left_valid = false;
        } else {
            left_valid = false;
        }
    }

    Vector2i tgt_top;
    VectorXf* tgt_top_patch;
    VectorXf* tgt_top_stylized_patch;
    if (top_valid) {
        tgt_top = NNF[pos_to_index(xy - Vector2i(0, 1), tgt.width)];
        tmp = xy + tgt_top;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_top_index = pos_to_index(tmp, tgt.width);
            tgt_top_patch = &(tgt.patches_original[tgt_top_index]->buffer);
            tgt_top_stylized_patch = &(tgt.patches_stylized[tgt_top_index]->buffer);
            if (tgt.patches_original[tgt_top_index]->is_matched) top_valid = false;
        } else {
            top_valid = false;
        }
    }

    double current_dist = Energy(src_patch, tgt_center_patch, src_stylized_patch, tgt_center_stylized_patch, this->mu);
    if (left_valid) {
        double left_dist = Energy(src_patch, *tgt_left_patch, src_stylized_patch, *tgt_left_stylized_patch, this->mu);
        if (left_dist < current_dist) {
            NNF[src_index] = tgt_left;
            errors[src_index] = std::make_pair(src_index, left_dist);
        }
    }

    if (top_valid) {
        double top_dist = Energy(src_patch, *tgt_top_patch, src_stylized_patch, *tgt_top_stylized_patch, this->mu);
        if (top_dist < current_dist) {
            NNF[src_index] = tgt_top;
            errors[src_index] = std::make_pair(src_index, top_dist);
        }
    }
}

void Patchmatcher::propagate_even(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool right_valid = (xy[0] < src.width - 1);
    bool bottom_valid = (xy[1] < src.height - 1);

    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_patch = src.patches_original[src_index]->buffer;
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_patch = tgt.patches_original[tgt_center_index]->buffer;
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;

    Vector2i tgt_right;
    Vector2i tmp;
    VectorXf* tgt_right_patch;
    VectorXf* tgt_right_stylized_patch;
    if (right_valid) {
        tgt_right = NNF[pos_to_index(xy + Vector2i(1, 0), tgt.width)];
        tmp = xy + tgt_right;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_right_index = pos_to_index(tmp, tgt.width);
            tgt_right_patch = &(tgt.patches_original[tgt_right_index]->buffer);
            tgt_right_stylized_patch = &(tgt.patches_stylized[tgt_right_index]->buffer);
            if (tgt.patches_original[tgt_right_index]->is_matched) right_valid = false;
        } else {
            right_valid = false;
        }
    }

    Vector2i tgt_bottom;
    VectorXf* tgt_bottom_patch;
    VectorXf* tgt_bottom_stylized_patch;
    if (bottom_valid) {
        tgt_bottom = NNF[pos_to_index(xy + Vector2i(0, 1), tgt.width)];
        tmp = xy + tgt_bottom;
        if ((tmp[0] >= 0) && (tmp[0] < tgt.width) && (tmp[1] >= 0) && (tmp[1] < tgt.height)) {
            int tgt_bottom_index = pos_to_index(tmp, tgt.width);
            tgt_bottom_patch = &(tgt.patches_original[tgt_bottom_index]->buffer);
            tgt_bottom_stylized_patch = &(tgt.patches_stylized[tgt_bottom_index]->buffer);
            if (tgt.patches_original[tgt_bottom_index]->is_matched) bottom_valid = false;
        } else {
            bottom_valid = false;
        }
    }

    double current_dist = Energy(src_patch, tgt_center_patch, src_stylized_patch, tgt_center_stylized_patch, this->mu);
    if (right_valid) {
        double right_dist = Energy(src_patch, *tgt_right_patch, src_stylized_patch, *tgt_right_stylized_patch, this->mu);
        if (right_dist < current_dist) {
            NNF[src_index] = tgt_right;
            errors[src_index] = std::make_pair(src_index, right_dist);
        }
    }

    if (bottom_valid) {
        double bottom_dist = Energy(src_patch, *tgt_bottom_patch, src_stylized_patch, *tgt_bottom_stylized_patch, this->mu);
        if (bottom_dist < current_dist) {
            NNF[src_index] = tgt_bottom;
            errors[src_index] = std::make_pair(src_index, bottom_dist);
        }
    }
}

NNF_t Patchmatcher::patch_match(const Image& src, const Image& tgt, std::unordered_set<int>& unmatched) {
    // fill NNF randomly
    NNF_t NNF;
    int imgSize = src.patches_original.size();
    int numIterations = 6;

    randomize_NNF(NNF, src, tgt, unmatched);

    for (int iteration = 1; iteration <= numIterations; ++iteration) {
        if (iteration % 2) {
            // scanline order
            for (int j = 0; j < imgSize; ++j) {
                const Vector2i& xy = src.patches_original[j]->coordinates;

                // propagation
                propagate_odd(NNF, src, tgt, xy);

                // random search
                random_search(NNF, src, tgt, xy);
            }

        } else {
            // reverse scanline order
            for (int j = imgSize - 1; j >= 0; --j) {
                const Vector2i& xy = src.patches_original[j]->coordinates;

                // propagation
                propagate_even(NNF, src, tgt, xy);

                // random search
                random_search(NNF, src, tgt, xy);
            }

        }
    }

    return NNF;
}

std::vector<RGBA> recreate_image(NNF_t NNF, const Image& target) {
    std::vector<RGBA> reconstruction(target.width * target.height);
    for (int i = 0; i < reconstruction.size(); ++i) {
        double r, g, b;
        r = target.patches_original[pos_to_index(index_to_position(i, target.width) + NNF[i], target.width)]->buffer[36];
        g = target.patches_original[pos_to_index(index_to_position(i, target.width) + NNF[i], target.width)]->buffer[37];
        b = target.patches_original[pos_to_index(index_to_position(i, target.width) + NNF[i], target.width)]->buffer[38];
        reconstruction[i] = RGBA{ float_to_int(r), float_to_int(g), float_to_int(b) };
    }
    return reconstruction;
}
