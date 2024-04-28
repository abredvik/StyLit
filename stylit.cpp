#include "stylit.h"

#include <iostream>
#include <fstream>

Stylit::Stylit()
{

    //TO DO: initialize stuff

}

void init_image(const std::vector<QString>& filenames, Image& img, int num_iterations) {
//#pragma omp parallel for
    Gaussianpyramid gaussianpyramid;
    img.pyramid = new Pyramid_t();
    for (int i = 0; i < filenames.size(); ++i) {
        const QString& filename  = filenames[i];
        auto tup = loadImageFromFile(filename);
        const std::vector<RGBA>& RGBimage = std::get<0>(tup);
        img.width = std::get<1>(tup);
        img.height = std::get<2>(tup);
        //const std::vector<Patch*>& patches = get_patches(RGBimage, img.width, img.height);

        switch (i) {
        case 0: //img.patches_original = patches;
            img.pyramid->color = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        case 1: //img.patches_LPE1 = patches;
            img.pyramid->LPE1 = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        case 2: //img.patches_LPE2 = patches;
            img.pyramid->LPE2 = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        case 3: //img.patches_LPE3 = patches;
            img.pyramid->LPE3 = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        case 4:
            img.pyramid->LPE4 = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        case 5: //img.patches_stylized = patches;
            img.pyramid->style = gaussianpyramid.create_pyrimid(num_iterations, RGBimage, img.width, img.height);
            break;
        }
    }
}

std::vector<RGBA> Stylit::run(Image& src, Image& tgt, int iterations){
    Scale scale;

    int original_src_width = src.width;
    int original_src_height = src.height;

    int original_tgt_width = src.width;
    int original_tgt_height = src.height;

    for(int i = 0; i < iterations; i++){

        src.width = original_src_width / (1 << (iterations - i - 1));
        src.height = original_src_height / (1 << (iterations - i - 1));

        tgt.width = original_tgt_width / (1 << (iterations - i - 1));
        tgt.height = original_tgt_height / (1 << (iterations - i - 1));

        src.patches_original = get_patches(src.pyramid->color[i], src.width, src.height);
        src.patches_LPE1 = get_patches(src.pyramid->LPE1[i], src.width, src.height);
        src.patches_LPE2 = get_patches(src.pyramid->LPE2[i], src.width, src.height);
        src.patches_LPE3 = get_patches(src.pyramid->LPE3[i], src.width, src.height);
        src.patches_LPE4 = get_patches(src.pyramid->LPE4[i], src.width, src.height);
        src.patches_stylized = get_patches(src.pyramid->style[i], src.width, src.height);

        tgt.patches_original = get_patches(tgt.pyramid->color[i], tgt.width, tgt.height);
        tgt.patches_LPE1 = get_patches(tgt.pyramid->LPE1[i], tgt.width, tgt.height);
        tgt.patches_LPE2 = get_patches(tgt.pyramid->LPE2[i], tgt.width, tgt.height);
        tgt.patches_LPE3 = get_patches(tgt.pyramid->LPE3[i], tgt.width, tgt.height);
        tgt.patches_LPE4 = get_patches(tgt.pyramid->LPE4[i], tgt.width, tgt.height);
        tgt.patches_stylized = get_patches(tgt.pyramid->style[i], tgt.width, tgt.height);

        stylit_algorithm(src, tgt, i);

        if(i != iterations - 1){
            tgt.pyramid->style[i + 1] = scale.handle_scale(tgt.pyramid->style[i], tgt.width, tgt.height, 2, 2);
        }

        std::cout << "done with iteration" << std::endl;
    }

//    std::vector<RGBA> output(tgt.patches_stylized.size());

//    for(int j = 0; j < tgt.patches_stylized.size(); ++j){
//        float r, g, b;
//        r = tgt.patches_stylized[j]->buffer[36];
//        g = tgt.patches_stylized[j]->buffer[37];
//        b = tgt.patches_stylized[j]->buffer[38];
//        output[j] = toRGBA(Vector3f(r, g, b));
//    }

    return tgt.pyramid->style[iterations - 1];
}

void Stylit::stylit_algorithm(const Image& src, Image& tgt, int current_iteration){

    int targets_covered = 0;

    float percent_coverage = 0.f;

    Patchmatcher patchmatcher(src.width, src.height);
    int total_targets = tgt.patches_original.size();

    std::unordered_set<int> unmatched;
    for (int i = 0; i < tgt.patches_original.size(); ++i) {
        tgt.patches_original[i]->is_matched = false;
        tgt.patches_LPE1[i]->is_matched = false;
        tgt.patches_LPE2[i]->is_matched = false;
        tgt.patches_LPE3[i]->is_matched = false;
        tgt.patches_LPE4[i]->is_matched = false;
        tgt.patches_stylized[i]->is_matched = false;
        unmatched.insert(i);
    }

    while(percent_coverage < 0.95f){
        NNF_t temp_NNF = patchmatcher.patch_match(src, tgt, unmatched);
        int k = 0.7 * patchmatcher.errors.size();
        //int k = calculate_error_budget(patchmatcher.errors);
        std::cout << k << std::endl;

        for (int i = 0; i < std::min((int)patchmatcher.errors.size(), k); i++){
            int source_index = patchmatcher.errors[i].first;

            int target_index = pos_to_index(temp_NNF[source_index] + index_to_position(source_index, src.width), tgt.width);

            if(tgt.patches_original[target_index]->is_matched){
                continue;
            }

            targets_covered++;
            unmatched.erase(target_index);
            tgt.patches_original[target_index]->is_matched = true;
            tgt.patches_LPE1[target_index]->is_matched = true;
            tgt.patches_LPE2[target_index]->is_matched = true;
            tgt.patches_LPE3[target_index]->is_matched = true;
            tgt.patches_LPE4[target_index]->is_matched = true;
            final_NNF[source_index] = temp_NNF[source_index];
            final_reverse_NNF[target_index] = -temp_NNF[source_index];

        percent_coverage = (float) targets_covered / (float) total_targets;

        }

        std::cout << percent_coverage << std::endl;

    }
    resolve_unmatched(src, tgt, unmatched);

    std::vector<RGBA> new_image(tgt.width * tgt.height);
#pragma omp parallel for
    for(int i = 0; i < (tgt.width * tgt.height); i ++){
        if (tgt.patches_original[i]->is_matched){
            new_image[i] = average(i, src, tgt);
//            Vector2i offset = final_reverse_NNF[i];
//            Vector2i xy = index_to_position(i, tgt.width);
//            int source_index = pos_to_index(xy + offset, src.width);
//            float r, g, b;
//            r = g = b = 0;
//            r = src.patches_stylized[source_index]->buffer[36];
//            g = src.patches_stylized[source_index]->buffer[37];
//            b = src.patches_stylized[source_index]->buffer[38];
//            new_image[i] = toRGBA(Vector3f(r, g, b));
        }
    }

    tgt.pyramid->style[current_iteration] = new_image;

    std::cout << "done with stylit algorithm" << std::endl;

//    tgt.patches_stylized = get_patches(new_image, tgt.width, tgt.height);

}

struct compare {
    inline bool operator() (const std::pair<int, double> &pair1, const std::pair<int, double> &pair2) {
        return pair1.second < pair2.second;
    }
};

int Stylit::calculate_error_budget(std::vector<std::pair<int, double>> &errors){
    std::sort(errors.begin(), errors.end(), compare());
    int num_steps = std::min((int)errors.size(), (int)50);
    int step_size = (errors.size()  + num_steps - 1) / num_steps;
    MatrixX2d l_matrix(num_steps, 2);
    VectorXd b_vector(num_steps);
    double max_error = errors[errors.size() - 1].second;
#pragma omp parallel for
    for (int i = 0; i < errors.size(); i += step_size) {
        l_matrix(i / step_size, 0) = 1.0;
        l_matrix(i / step_size, 1) = -1.0 * ((i / step_size) / (float)num_steps);
        b_vector(i / step_size) = 1.0 / (errors[i].second / max_error);
    }

    Vector2d result = l_matrix.colPivHouseholderQr().solve(b_vector);
    double a = result(0);
    double b = result(1);
    int k = errors.size() * (-sqrt(1.0 / b) + (a / b));

    return k;
}

void Stylit::resolve_unmatched(const Image& src, Image& tgt, const std::unordered_set<int>& unmatched) {
    for (int i = 0; i < tgt.patches_original.size(); ++i) {
        if (!tgt.patches_original[i]->is_matched) {
            std::vector<VectorXf*> tgt_patches(5);
            tgt_patches[0] = &(tgt.patches_original[i]->buffer);
            tgt_patches[1] = &(tgt.patches_LPE1[i]->buffer);
            tgt_patches[2] = &(tgt.patches_LPE2[i]->buffer);
            tgt_patches[3] = &(tgt.patches_LPE3[i]->buffer);
            tgt_patches[4] = &(tgt.patches_LPE4[i]->buffer);
            const VectorXf& tgt_style = tgt.patches_stylized[i]->buffer;

            Vector2i nearest_neighbor_offset = nearest_neighbor(src, tgt_patches, tgt_style, index_to_position(i, tgt.width));
            int source_index = pos_to_index(index_to_position(i, tgt.width) - nearest_neighbor_offset, src.width);

            tgt.patches_original[i]->is_matched = true;
            tgt.patches_LPE1[i]->is_matched = true;
            tgt.patches_LPE2[i]->is_matched = true;
            tgt.patches_LPE3[i]->is_matched = true;
            tgt.patches_LPE4[i]->is_matched = true;
            final_NNF[source_index] = nearest_neighbor_offset;
            final_reverse_NNF[i] = -final_NNF[source_index];
        }
    }
}

Vector2i Stylit::nearest_neighbor(const Image& src, std::vector<VectorXf*> tgt_patches, const VectorXf& tgt_style, Vector2i xy) {
    double min_energy = std::numeric_limits<double>::infinity();
    Vector2i nearest;
#pragma omp parallel for
    for (int i = 0; i < src.patches_original.size(); ++i) {
        std::vector<VectorXf*> src_patches(5);
        const VectorXf& src_stylized_patch = src.patches_stylized[i]->buffer;
        src_patches[0] = &(src.patches_original[i]->buffer);
        src_patches[1] = &(src.patches_LPE1[i]->buffer);
        src_patches[2] = &(src.patches_LPE2[i]->buffer);
        src_patches[3] = &(src.patches_LPE3[i]->buffer);
        src_patches[4] = &(src.patches_LPE4[i]->buffer);

        double energy = Energy(src_patches, tgt_patches, src_stylized_patch, tgt_style, 2.0);
        if (energy < min_energy) {
            nearest = xy - index_to_position(i, src.width);
            min_energy = energy;
        }
    }

    return nearest;
}

RGBA Stylit::average(int index, const Image& src, Image& tgt){

    float r, g, b;
    r = 0;
    g = 0;
    b = 0;
    int j = 24;
    for(int neighbor_index : tgt.patches_original[index]->neighbor_patches){
        Vector2i offset = final_reverse_NNF[neighbor_index];
        Vector2i xy = index_to_position(neighbor_index, tgt.width);
        int source_index = pos_to_index(xy + offset, src.width);
        // Might be causing issues for edge pixels
        r += src.patches_stylized[source_index]->buffer[j * 3];
        g += src.patches_stylized[source_index]->buffer[(j * 3) + 1];
        b += src.patches_stylized[source_index]->buffer[(j * 3) + 2];
        --j;
    }
    r = r / tgt.patches_original[index]->neighbor_patches.size();
    g = g / tgt.patches_original[index]->neighbor_patches.size();
    b = b / tgt.patches_original[index]->neighbor_patches.size();

    return toRGBA(Vector3f(r, g, b));

//    tgt.patches_stylized[index]->buffer[3] = VectorXf::Zero(tgt.patches_stylized[index]->buffer.size());
//    for(int neighbor_index : tgt.patches_original[index]->neighbor_patches){
//        Vector2i offset = final_reverse_NNF[neighbor_index];
//        Vector2i xy = index_to_position(neighbor_index, tgt.width);
//        int source_index = pos_to_index(xy + offset, src.width);
//        tgt.patches_stylized[index]->buffer += src.patches_stylized[source_index]->buffer;
//    }
//    tgt.patches_stylized[index]->buffer = tgt.patches_stylized[index]->buffer / (float) tgt.patches_original[index]->neighbor_patches.size();



//    Vector2i offset = final_reverse_NNF[index];
//    Vector2i xy = index_to_position(index, tgt.width);
//    int source_index = pos_to_index(xy + offset, src.width);
//    tgt.patches_stylized[index]->buffer = src.patches_stylized[source_index]->buffer;

}

