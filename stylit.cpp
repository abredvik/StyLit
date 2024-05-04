#include "stylit.h"
#include "patchmatch.h"

Stylit::Stylit()
{

    //TO DO: initialize stuff

}

void init_image(const std::vector<QString>& LPEnames, std::vector<RGBA> style_image, Image& img, int num_iterations) {
//#pragma omp parallel for
    Gaussianpyramid gaussianpyramid;
    img.pyramid = new Pyramid_t();
    img.pyramid->LPEs.resize(num_iterations);
    for (int i = 0; i < num_iterations; ++i) {
        img.pyramid->LPEs[i].resize(LPEnames.size());
    }

    // do gaussian pyramid for stylized image
    img.pyramid->style =  gaussianpyramid.create_pyrimid(num_iterations, style_image, img.width, img.height);

    // do gaussian pyramids
    for (int i = 0; i < LPEnames.size(); ++i) {
        const QString& filename  = LPEnames[i];
        auto LPEtup = loadImageFromFile(filename);
//        img.width = std::get<1>(LPEtup);
//        img.height = std::get<2>(LPEtup);
        const std::vector<std::vector<RGBA>>& pyr = gaussianpyramid.create_pyrimid(num_iterations, *std::get<0>(LPEtup), img.width, img.height);
        for (int j = 0; j < pyr.size(); ++j) {
            img.pyramid->LPEs[j][i] = pyr[j];
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

        auto src_orig_style = get_patches(src.pyramid->LPEs[i], src.pyramid->style[i], this->window_width, src.width, src.height);
        src.patches_original = src_orig_style.first;
        src.patches_stylized = src_orig_style.second;

        auto tgt_orig_style = get_patches(tgt.pyramid->LPEs[i], tgt.pyramid->style[i], this->window_width, tgt.width, tgt.height);
        tgt.patches_original = tgt_orig_style.first;
        tgt.patches_stylized = tgt_orig_style.second;

        stylit_algorithm(src, tgt, i);

        if(i != iterations - 1){
            tgt.pyramid->style[i + 1] = scale.handle_scale(tgt.pyramid->style[i], tgt.width, tgt.height, 2, 2);
        }

        std::cout << "done with iteration" << std::endl;
    }

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
        tgt.patches_stylized[i]->is_matched = false;
        unmatched.insert(i);
    }

    double T = 0.0;

    while(percent_coverage < 0.95f){
        NNF_t temp_NNF = patchmatcher.patch_match(src, tgt, unmatched);
        size_t k;
        if (T == 0.0) {
            std::pair<int, double> k_T = calculate_k_and_error_budget(patchmatcher.errors);
            k = k_T.first;
            T = k_T.second;
        } else {
            k = calculate_k(patchmatcher.errors);
        }

        //std::cout << "k = " << k << " < " << patchmatcher.errors.size() << " = errors.size()" << std::endl;


        std::unordered_set<int> matched_target_indices;

        double sum_errors = 0.0;
        for (int i = 0; i < (int)std::min(k, patchmatcher.errors.size()); i++){
            sum_errors += patchmatcher.errors[i].second;
            if (sum_errors > T) {
                //std::cout << "error budget reached at i = " << i << std::endl;
                break;
            }

            int source_index = patchmatcher.errors[i].first;

            int target_index = pos_to_index(temp_NNF[source_index] + index_to_position(source_index, src.width), tgt.width);
            matched_target_indices.insert(target_index);

            if(tgt.patches_original[target_index]->is_matched){
                continue;
            }

            targets_covered++;
            unmatched.erase(target_index);
            tgt.patches_original[target_index]->is_matched = true;
            tgt.patches_stylized[target_index]->is_matched = true;
            final_NNF[source_index] = temp_NNF[source_index];
            final_reverse_NNF[target_index] = -temp_NNF[source_index];

        }

        //std::cout << "number of unique targets matched: " << matched_target_indices.size() << std::endl;
        //std::cout << "targets covered: " << targets_covered << std::endl;
        percent_coverage = (float) targets_covered / (float) total_targets;

        std::cout << percent_coverage << std::endl;

    }
    resolve_unmatched(src, tgt, unmatched);

    std::vector<RGBA> new_image(tgt.width * tgt.height);
#pragma omp parallel for
    for(int i = 0; i < (tgt.width * tgt.height); i ++){
        if (tgt.patches_original[i]->is_matched){
            new_image[i] = average(i, src, tgt);
        }
    }

    tgt.pyramid->style[current_iteration] = new_image;

    //std::cout << "done with stylit algorithm" << std::endl;

}

struct compare {
    inline bool operator() (const std::pair<int, double> &pair1, const std::pair<int, double> &pair2) {
        return pair1.second < pair2.second;
    }
};

int Stylit::calculate_k(std::vector<std::pair<int, double>> &errors){
    std::sort(errors.begin(), errors.end(), compare());
    int num_steps = std::min((int)errors.size(), (int)50);
    int step_size = (errors.size()  + num_steps - 1) / num_steps;
    MatrixX2d l_matrix(num_steps, 2);
    VectorXd b_vector(num_steps);
    double max_error = errors[errors.size() - 1].second;
    for (int i = 0; i < errors.size(); i += step_size) {
        l_matrix(i / step_size, 0) = 1.0;
        l_matrix(i / step_size, 1) = -1.0 * ((i / step_size) / (double)num_steps);
        b_vector(i / step_size) = 1.0 / (errors[i].second / max_error);
    }

    Vector2d result = l_matrix.colPivHouseholderQr().solve(b_vector);
    double a = result(0);
    double b = result(1);
    int k = errors.size() * (-sqrt(1.0 / b) + (a / b));

    return k;
}

std::pair<int, double> Stylit::calculate_k_and_error_budget(std::vector<std::pair<int, double>> &errors){
    int k = calculate_k(errors);
    double T = 0.0;
    for (int i = 0; i < k; ++i) {
        T += errors[i].second;
    }

    return std::make_pair(k, T);
}

void Stylit::resolve_unmatched(const Image& src, Image& tgt, const std::unordered_set<int>& unmatched) {
    for (int i = 0; i < tgt.patches_original.size(); ++i) {
        if (!tgt.patches_original[i]->is_matched) {
            const VectorXf& tgt_patch = tgt.patches_original[i]->buffer;
            const VectorXf& tgt_style = tgt.patches_stylized[i]->buffer;

            Vector2i nearest_neighbor_offset = nearest_neighbor(src, tgt_patch, tgt_style, index_to_position(i, tgt.width));
            int source_index = pos_to_index(index_to_position(i, tgt.width) - nearest_neighbor_offset, src.width);

            tgt.patches_original[i]->is_matched = true;
            final_NNF[source_index] = nearest_neighbor_offset;
            final_reverse_NNF[i] = -final_NNF[source_index];
        }
    }
}

Vector2i Stylit::nearest_neighbor(const Image& src, const VectorXf& tgt_patch, const VectorXf& tgt_style, Vector2i xy) {
    double min_energy = std::numeric_limits<double>::infinity();
    Vector2i nearest;
#pragma omp parallel for
    for (int i = 0; i < src.patches_original.size(); ++i) {
        const VectorXf& src_patch = src.patches_original[i]->buffer;
        const VectorXf& src_stylized_patch = src.patches_stylized[i]->buffer;

        double energy = Energy(src_patch, tgt_patch, src_stylized_patch, tgt_style, this->mu);
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
}

