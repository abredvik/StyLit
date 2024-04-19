#include "patchmatch.h"

#include <random>

//Patchmatcher::Patchmatcher()
//{

//}


//NNF_t Patchmatcher::patch_match(){
//}

std::vector<VectorXf> get_patches(const std::vector<RGBA>& img, int width, int height) {
    // return a bunch of 5 by 5 (or representation of) patches
    // one for each pixel
    std::vector<VectorXf> result;
    result.reserve(img.size());
    int window_width = 5;
    int w = window_width / 2;

    for (int p = 0; p < img.size(); ++p) {
        Vector2i xy = index_to_position(p, width);
        int row = xy[1], col = xy[0];
        VectorXf patch(3 * window_width * window_width);
        for (int i = row - w; i < row + w; ++i) {
            for (int j = col - w; j < col + w; ++j) {
                int patch_index = 3 * pos_to_index(Vector2i(j - col + w, i - row + w), window_width);

                if (i < 0 || i >= height || j < 0 || j >= width) {
                    const RGBA& center = img[pos_to_index(Vector2i(col, row), width)];
                    patch[patch_index] = uint8_to_float(center.r);
                    patch[patch_index + 1] = uint8_to_float(center.g);
                    patch[patch_index + 2] = uint8_to_float(center.b);
                    continue;
                }

                int pixel_index = pos_to_index(Vector2i(j, i), width);
                const RGBA& rgb = img[pixel_index];
                patch[patch_index] = uint8_to_float(rgb.r);
                patch[patch_index + 1] = uint8_to_float(rgb.g);
                patch[patch_index + 2] = uint8_to_float(rgb.b);
            }
        }

        result.push_back(patch);
    }

    return result;

}

double Distance(VectorXf A, VectorXf B) {
//    return (|| A'(p) - B'(q) ||)^2 + mu (|| A(p) - B(q) ||)^2
    return (B - A).norm();
}

std::random_device                      rand_dev;
std::mt19937                            generator(rand_dev());
std::uniform_real_distribution<double>   distr(-1.0, 1.0);

void random_search(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width){
    Vector2i v0 = NNF[pos_to_index(xy, width)]; // this is the offset for the patch of interest
    VectorXf final_patch = tgt_patches[pos_to_index(xy + v0, width)];
    double alpha = 0.5;
    int i = 0;
    int w = width;

    while (w * std::pow(alpha, i) > 1) {
        Vector2d R(distr(generator), distr(generator));
        Vector2i ui = (v0.cast<double>() + (w * std::pow(alpha, i) * R)).cast<int>();
        Vector2i newCoord = xy + ui;
        newCoord = newCoord.cwiseMax(0).cwiseMin(width - 1);
        VectorXf candidate_patch = tgt_patches[pos_to_index(newCoord, width)];
        if (Distance(src_patch, candidate_patch) < Distance(src_patch, final_patch)){
            v0 = newCoord - xy;
            final_patch = candidate_patch;
        }

        ++i;
    }

    NNF[pos_to_index(xy, width)] = v0;
}

void randomize_NNF(NNF_t& NNF, int imgSize, int width, int height) {
    for (int i = 0; i < imgSize; ++i) {
        Vector2i xy = index_to_position(i, width);
        Vector2i newXY(rand() % width, rand() % height);
        NNF[i] = (newXY - xy);
    }
}

void propagate_odd(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width, int height) {
    bool left_valid = (xy[0] > 0);
    bool top_valid = (xy[1] > 0);

    Vector2i tgt_center = NNF[pos_to_index(xy, width)];
    VectorXf tgt_center_patch = tgt_patches[pos_to_index(xy + tgt_center, width)];

    Vector2i tgt_left;
    Vector2i tmp;
    VectorXf tgt_left_patch;
    if (left_valid) {
        tgt_left = NNF[pos_to_index(xy - Vector2i(1, 0), width)];
        tmp = xy + tgt_left;
        if ((tmp[0] >= 0) && (tmp[0] < width) && (tmp[1] >= 0) && (tmp[1] < height)) {
            tgt_left_patch = tgt_patches[pos_to_index(tmp, width)];
        } else {
            left_valid = false;
        }
    }

    Vector2i tgt_top;
    VectorXf tgt_top_patch;
    if (top_valid) {
        tgt_top = NNF[pos_to_index(xy - Vector2i(0, 1), width)];
        tmp = xy + tgt_top;
        if ((tmp[0] >= 0) && (tmp[0] < width) && (tmp[1] >= 0) && (tmp[1] < height)) {
            tgt_top_patch = tgt_patches[pos_to_index(tmp, width)];
        } else {
            top_valid = false;
        }
    }

    double current_dist = Distance(src_patch, tgt_center_patch);
    int index = pos_to_index(xy, width);
    if (left_valid && (Distance(src_patch, tgt_left_patch) < current_dist)) {
        NNF[index] = tgt_left;
    }

    if (top_valid && (Distance(src_patch, tgt_top_patch) < current_dist)) {
        NNF[index] = tgt_top;
    }
}

void propagate_even(NNF_t& NNF, const VectorXf& src_patch, const std::vector<VectorXf>& tgt_patches, Vector2i xy, int width, int height) {
    bool right_valid = (xy[0] < width - 1);
    bool bottom_valid = (xy[1] < height - 1);

    Vector2i tgt_center = NNF[pos_to_index(xy, width)];
    VectorXf tgt_center_patch = tgt_patches[pos_to_index(xy + tgt_center, width)];

    Vector2i tgt_right;
    Vector2i tmp;
    VectorXf tgt_right_patch;
    if (right_valid) {
        tgt_right = NNF[pos_to_index(xy + Vector2i(1, 0), width)];
        tmp = xy + tgt_right;
        if ((tmp[0] >= 0) && (tmp[0] < width) && (tmp[1] >= 0) && (tmp[1] < height)) {
            tgt_right_patch = tgt_patches[pos_to_index(tmp, width)];
        } else {
            right_valid = false;
        }
    }

    Vector2i tgt_bottom;
    VectorXf tgt_bottom_patch;
    if (bottom_valid) {
        tgt_bottom = NNF[pos_to_index(xy + Vector2i(0, 1), width)];
        tmp = xy + tgt_bottom;
        if ((tmp[0] >= 0) && (tmp[0] < width) && (tmp[1] >= 0) && (tmp[1] < height)) {
            tgt_bottom_patch = tgt_patches[pos_to_index(tmp, width)];
        } else {
            bottom_valid = false;
        }
    }

    double current_dist = Distance(src_patch, tgt_center_patch);
    int index = pos_to_index(xy, width);
    if (right_valid && (Distance(src_patch, tgt_right_patch) < current_dist)) {
        // offset(x,y) = offset(x-1,y)
        NNF[index] = tgt_right;
    }

    if (bottom_valid && (Distance(src_patch, tgt_bottom_patch) < current_dist)) {
        // offset(x,y) = offset(x,y-1)
        NNF[index] = tgt_bottom;
    }
}

NNF_t patch_match(const std::vector<VectorXf>& src_patches, const std::vector<VectorXf>& tgt_patches, int width, int height) {
    // fill NNF randomly
    NNF_t NNF;
    int imgSize = src_patches.size();
    int numIterations = 6;

    randomize_NNF(NNF, imgSize, width, height);

    for (int iteration = 1; iteration <= numIterations; ++iteration) {
        if (iteration % 2) {
            // scanline order
            for (int j = 0; j < imgSize; ++j) {
                Vector2i xy = index_to_position(j, width);
                VectorXf src_patch = src_patches[j];

                // propagation
                propagate_odd(NNF, src_patch, tgt_patches, xy, width, height);

                // random search
                random_search(NNF, src_patch, tgt_patches, xy, width);
            }

        } else {
            // reverse scanline order
            for (int j = imgSize - 1; j >= 0; --j) {
                Vector2i xy = index_to_position(j, width);
                VectorXf src_patch = src_patches[j];

                // propagation
                propagate_even(NNF, src_patch, tgt_patches, xy, width, height);

                // random search
                random_search(NNF, src_patch, tgt_patches, xy, width);
            }

        }
    }

    return NNF;
}

std::vector<RGBA> recreate_image(NNF_t NNF, const std::vector<RGBA>& target, int width) {
    std::vector<RGBA> reconstruction(target.size());
    for (int i = 0; i < target.size(); ++i) {
        reconstruction[i] = target[pos_to_index(index_to_position(i, width) + NNF[i], width)];
    }
    return reconstruction;
}

