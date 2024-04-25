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

void verify_patch(Patch *patch) {
    if (patch->buffer.size() != 75) throw std::runtime_error("invalid buffer size");
    for (int i = 0; i < patch->buffer.size(); ++i) {
        if (patch->buffer[i] > 1.0) {
            std::cout << "value: " << patch->buffer[i] << std::endl;
            std::cout << "patch: " << patch->buffer << std::endl;
            throw std::runtime_error("invalid value encountered");
        }
    }
}

std::vector<Patch*> get_patches(const std::vector<RGBA>& img, int width, int height) {
    // return a bunch of 5 by 5 (or representation of) patches
    // one for each pixel
    std::vector<Patch*> result(img.size());
//    result.reserve(img.size());
    int window_width = 5;
    int w = window_width / 2;

#pragma omp parallel for
    for (int p = 0; p < img.size(); ++p) {
        Patch *patch = new Patch();
        patch->buffer = VectorXf(3 * window_width * window_width);
        patch->coordinates = index_to_position(p, width);
        patch->neighbor_patches.reserve(window_width * window_width);
        patch->is_matched = false;

        const Vector2i& xy = patch->coordinates;
        int row = xy[1], col = xy[0];

        std::unordered_set<int> seen_indices;

        for (int i = row - w; i <= row + w; ++i) {
            for (int j = col - w; j <= col + w; ++j) {
                int patch_index = 3 * pos_to_index(Vector2i(j - col + w, i - row + w), window_width);
//                std::cout << "patch index: " << patch_index << std::endl;
                seen_indices.insert(patch_index);
                seen_indices.insert(patch_index+1);
                seen_indices.insert(patch_index+2);
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
//                if (patch->buffer[patch_index] > 10000 || patch->buffer[patch_index + 1] > 10000 || patch->buffer[patch_index + 2] > 10000) {
//                    std::cout << "i: " << i << ", j: " << j << std::endl;
//                    std::cout << "patch index: " << patch_index << std::endl;
//                    std::cout << patch->buffer << std::endl;
//                    throw std::runtime_error("bad value");
//                }
            }
        }

        for (int ind = 0; ind < 75; ++ind) {
            if (seen_indices.find(ind) == seen_indices.end()) {
                std::cout << "does not contain: " << ind << std::endl;
            }
        }
        if (seen_indices.size() != 75) throw std::runtime_error("not 75");

//        std::cout << "get_patches:" << std::endl;
//        verify_patch(patch);

        result[p] = patch;
    }

    return result;

}

double Distance(const VectorXf& A, const VectorXf& B) {
//    return (|| A'(p) - B'(q) ||)^2 + mu (|| A(p) - B(q) ||)^2
    return (B - A).squaredNorm();
}

double Energy(const std::vector<VectorXf*>& A, const std::vector<VectorXf*>& B,
              const VectorXf& Aprime, const VectorXf& Bprime, double mu) {
    double result = 0.0;
    for (int i = 0; i < 4; ++i) {
        result += mu * Distance(*A[i], *B[i]);
    }
    return result + Distance(Aprime, Bprime);
}

std::random_device                      rand_dev;
std::mt19937                            generator(rand_dev());
std::uniform_real_distribution<double>   distr(-1.0, 1.0);

void Patchmatcher::random_search(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy){
    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_original[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i v0 = NNF[pos_to_index(xy, src.width)]; // this is the offset for the patch of interest
    std::vector<VectorXf*> final_patches(4);
    int index = pos_to_index(xy + v0, tgt.width);
    const VectorXf& final_stylized_patch = tgt.patches_stylized[index]->buffer;
    final_patches[0] = &(tgt.patches_original[index]->buffer);
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
        candidate_patches[0] = &(tgt.patches_original[candidate_index]->buffer);
        candidate_patches[1] = &(tgt.patches_LPE1[candidate_index]->buffer);
        candidate_patches[2] = &(tgt.patches_LPE2[candidate_index]->buffer);
        candidate_patches[3] = &(tgt.patches_LPE3[candidate_index]->buffer);
        double candidate_energy = Energy(src_patches, candidate_patches, src_stylized_patch, candidate_stylized_patch, mu);

        if (candidate_energy < final_energy && !(tgt.patches_stylized[candidate_index]->is_matched)){
            v0 = newCoord - xy;
            final_patches = candidate_patches;
            final_energy = candidate_energy;
        }

        ++i;
    }

    NNF[src_index] = v0;
    errors[src_index] = std::make_pair(src_index, final_energy);
}

int random_pick(std::unordered_set<int>& available_patches) {
    auto it = available_patches.begin();
    int n = rand() % available_patches.size();
    std::advance(it, n);
    return *it;
}

void Patchmatcher::randomize_NNF(NNF_t& NNF, int srcSize, int srcWidth, int tgtWidth, std::unordered_set<int>& available_patches) {
    for (int i = 0; i < srcSize; ++i) {
        Vector2i xy = index_to_position(i, srcWidth);
        Vector2i newXY = index_to_position(random_pick(available_patches), tgtWidth);//(rand() % width, rand() % height);
        NNF[i] = (newXY - xy);
    }
}

void Patchmatcher::propagate_odd(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool left_valid = (xy[0] > 0);
    bool top_valid = (xy[1] > 0);

    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_original[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    std::vector<VectorXf*> tgt_center_patches(4);
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;
    tgt_center_patches[0] = &(tgt.patches_original[tgt_center_index]->buffer);
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
            tgt_left_patches[0] = &(tgt.patches_original[tgt_left_index]->buffer);
            tgt_left_patches[1] = &(tgt.patches_LPE1[tgt_left_index]->buffer);
            tgt_left_patches[2] = &(tgt.patches_LPE2[tgt_left_index]->buffer);
            tgt_left_patches[3] = &(tgt.patches_LPE3[tgt_left_index]->buffer);
            if (tgt.patches_original[tgt_left_index]->is_matched) left_valid = false;
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
            tgt_top_patches[0] = &(tgt.patches_original[tgt_top_index]->buffer);
            tgt_top_patches[1] = &(tgt.patches_LPE1[tgt_top_index]->buffer);
            tgt_top_patches[2] = &(tgt.patches_LPE2[tgt_top_index]->buffer);
            tgt_top_patches[3] = &(tgt.patches_LPE3[tgt_top_index]->buffer);
            if (tgt.patches_original[tgt_top_index]->is_matched) top_valid = false;
        } else {
            top_valid = false;
        }
    }

    double current_dist = Energy(src_patches, tgt_center_patches, src_stylized_patch, tgt_center_stylized_patch, mu);
    int index = pos_to_index(xy, src.width);
    if (left_valid) {
        double left_dist = Energy(src_patches, tgt_left_patches, src_stylized_patch, tgt_left_stylized_patch, mu);
        if (left_dist < current_dist) {
            NNF[index] = tgt_left;
            errors[index] = std::make_pair(index, left_dist);
        }
    }

    if (top_valid) {
        double top_dist = Energy(src_patches, tgt_top_patches, src_stylized_patch, tgt_top_stylized_patch, mu);
        if (top_dist < current_dist) {
            NNF[index] = tgt_top;
            errors[index] = std::make_pair(index, top_dist);
        }
    }
}

void Patchmatcher::propagate_even(NNF_t& NNF, const Image& src, const Image& tgt, Vector2i xy) {
    bool right_valid = (xy[0] < src.width - 1);
    bool bottom_valid = (xy[1] < src.height - 1);

    std::vector<VectorXf*> src_patches(4);
    int src_index = pos_to_index(xy, src.width);
    const VectorXf& src_stylized_patch = src.patches_stylized[src_index]->buffer;
    src_patches[0] = &(src.patches_original[src_index]->buffer);
    src_patches[1] = &(src.patches_LPE1[src_index]->buffer);
    src_patches[2] = &(src.patches_LPE2[src_index]->buffer);
    src_patches[3] = &(src.patches_LPE3[src_index]->buffer);

    Vector2i tgt_center = NNF[pos_to_index(xy, tgt.width)];
    std::vector<VectorXf*> tgt_center_patches(4);
    int tgt_center_index = pos_to_index(xy + tgt_center, tgt.width);
    const VectorXf& tgt_center_stylized_patch = tgt.patches_stylized[tgt_center_index]->buffer;
    tgt_center_patches[0] = &(tgt.patches_original[tgt_center_index]->buffer);
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
            tgt_right_patches[0] = &(tgt.patches_original[tgt_right_index]->buffer);
            tgt_right_patches[1] = &(tgt.patches_LPE1[tgt_right_index]->buffer);
            tgt_right_patches[2] = &(tgt.patches_LPE2[tgt_right_index]->buffer);
            tgt_right_patches[3] = &(tgt.patches_LPE3[tgt_right_index]->buffer);
            if (tgt.patches_original[tgt_right_index]->is_matched) right_valid = false;
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
            tgt_bottom_patches[0] = &(tgt.patches_original[tgt_bottom_index]->buffer);
            tgt_bottom_patches[1] = &(tgt.patches_LPE1[tgt_bottom_index]->buffer);
            tgt_bottom_patches[2] = &(tgt.patches_LPE2[tgt_bottom_index]->buffer);
            tgt_bottom_patches[3] = &(tgt.patches_LPE3[tgt_bottom_index]->buffer);
            if (tgt.patches_original[tgt_bottom_index]->is_matched) bottom_valid = false;
        } else {
            bottom_valid = false;
        }
    }

    double current_dist = Energy(src_patches, tgt_center_patches, src_stylized_patch, tgt_center_stylized_patch, mu);
    int index = pos_to_index(xy, src.width);
    if (right_valid) {
        double right_dist = Energy(src_patches, tgt_right_patches, src_stylized_patch, tgt_right_stylized_patch, mu);
        if (right_dist < current_dist) {
            NNF[index] = tgt_right;
            errors[index] = std::make_pair(index, right_dist);
        }
    }

    if (bottom_valid) {
        double bottom_dist = Energy(src_patches, tgt_bottom_patches, src_stylized_patch, tgt_bottom_stylized_patch, mu);
        if (bottom_dist < current_dist) {
            NNF[index] = tgt_bottom;
            errors[index] = std::make_pair(index, bottom_dist);
        }
    }
}

NNF_t Patchmatcher::patch_match(const Image& src, const Image& tgt, std::unordered_set<int>& available_patches) {
    // fill NNF randomly
    NNF_t NNF;
    int imgSize = src.patches_original.size();
    int numIterations = 6;

    randomize_NNF(NNF, imgSize, src.width, tgt.width, available_patches);
//    for (int i = 0; i < errors.size(); ++i) {
//        std::vector<VectorXf*> src_patches(4);
//        const VectorXf& src_stylized_patch = src.patches_stylized[i]->buffer;
//        src_patches[0] = &(src.patches_original[i]->buffer);
//        src_patches[1] = &(src.patches_LPE1[i]->buffer);
//        src_patches[2] = &(src.patches_LPE2[i]->buffer);
//        src_patches[3] = &(src.patches_LPE3[i]->buffer);

//        std::vector<VectorXf*> tgt_patches(4);
//        int tgt_index = pos_to_index(index_to_position(i, src.width) + NNF[i], tgt.width);
//        const VectorXf& tgt_stylized_patch = tgt.patches_stylized[tgt_index]->buffer;
//        tgt_patches[0] = &(tgt.patches_original[tgt_index]->buffer);
//        tgt_patches[1] = &(tgt.patches_LPE1[tgt_index]->buffer);
//        tgt_patches[2] = &(tgt.patches_LPE2[tgt_index]->buffer);
//        tgt_patches[3] = &(tgt.patches_LPE3[tgt_index]->buffer);

//        errors[i] = std::make_pair(i, Energy(src_patches, tgt_patches, src_stylized_patch, tgt_stylized_patch, 2));
//    }

//    return NNF;

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

