#include "stylit.h"

Stylit::Stylit()
{

    //TO DO: initialize stuff


}

void init_image(const std::vector<QString>& filenames, Image& img) {
//#pragma omp parallel for
    for (int i = 0; i < filenames.size(); ++i) {
        const QString& filename  = filenames[i];
        auto tup = loadImageFromFile(filename);
        const std::vector<RGBA>& RGBimage = std::get<0>(tup);
        img.width = std::get<1>(tup);
        img.height = std::get<2>(tup);
        const std::vector<Patch*>& patches = get_patches(RGBimage, img.width, img.height);

        switch (i) {
        case 0: img.patches_original = patches; break;
        case 1: img.patches_LPE1 = patches; break;
        case 2: img.patches_LPE2 = patches; break;
        case 3: img.patches_LPE3 = patches; break;
        case 4: img.patches_stylized = patches; break;
        }
    }
}

std::vector<RGBA> Stylit::run(const Image& src, Image& tgt, int iterations){
    for(int i = 0; i < iterations; i++){
        stylit_algorithm(src, tgt);
    }
    return output;
}

void Stylit::stylit_algorithm(const Image& src, Image& tgt){

    Patchmatcher patchmatcher(src.width, src.height);

    int targets_covered = 0;
    int total_targets = tgt.patches_original.size();
    float percent_coverage = 0.f;

    while(percent_coverage < 0.95f){
        NNF_t temp_NNF = patchmatcher.patch_match(src, tgt);
        int k = calculate_error_budget(patchmatcher.errors);

        for (int i = 0; i < k; i ++){
            int error_index = patchmatcher.errors[i].first;

            if(tgt.patches_original[error_index]->is_matched){
                continue;
            }

            targets_covered ++;
            tgt.patches_original[error_index]->is_matched = true;
            tgt.patches_LPE1[error_index]->is_matched = true;
            tgt.patches_LPE2[error_index]->is_matched = true;
            tgt.patches_LPE3[error_index]->is_matched = true;
            final_NNF[error_index] = temp_NNF[error_index];

        }

        percent_coverage = (float) targets_covered / (float) total_targets;
    }

    resolve_unmatched(src, tgt);

    for(int i = 0; i < (tgt.width * tgt.height); i ++){
        // TO DO: Write average()
        average(i);
    }

}

struct compare {
    inline bool operator() (const std::pair<int, double> &pair1, const std::pair<int, double> &pair2) {
        return pair1.second < pair2.second;
    }
};

int Stylit::calculate_error_budget(std::vector<std::pair<int, double>> &errors){
    std::sort(errors.begin(), errors.end(), compare());
    MatrixX2d l_matrix(errors.size(), 2);
    VectorXd b_vector(errors.size());
    double max_error = errors[errors.size() - 1].second;
    for (int i = 0; i < errors.size(); ++i) {
        l_matrix(i, 0) = 1.0;
        l_matrix(i, 1) = -1.0 * ((double)i / (double)errors.size());
        b_vector(i) = 1.0 / (errors[i].second / max_error);
    }

    Vector2d result = l_matrix.colPivHouseholderQr().solve(b_vector);
    double a = result(0);
    double b = result(1);
    int k = errors.size() * (sqrt(1.0 / b) + (a / b));

    return k;
}

void Stylit::resolve_unmatched(const Image& src, Image& tgt) {
    for(int i = 0; i < tgt.patches_original.size(); i++){
        if(!tgt.patches_original[i]->is_matched){
            // TO DO: write nearest_neighbor()
            std::vector<VectorXf*> tgt_patches(4);
            tgt_patches[0] = &(tgt.patches_original[i]->buffer);
            tgt_patches[1] = &(tgt.patches_LPE1[i]->buffer);
            tgt_patches[2] = &(tgt.patches_LPE2[i]->buffer);
            tgt_patches[3] = &(tgt.patches_LPE3[i]->buffer);
            const VectorXf& tgt_style = tgt.patches_stylized[i]->buffer;

            final_NNF[i] = nearest_neighbor(src, tgt_patches, tgt_style, index_to_position(i, tgt.width));
        }
    }
}

Vector2i Stylit::nearest_neighbor(const Image& src, std::vector<VectorXf*> tgt_patches, const VectorXf& tgt_style, Vector2i xy) {
    double min_energy = std::numeric_limits<double>::infinity();
    Vector2i nearest;

    for (int i = 0; i < src.patches_original.size(); ++i) {
        std::vector<VectorXf*> src_patches(4);
        const VectorXf& src_stylized_patch = src.patches_stylized[i]->buffer;
        src_patches[0] = &(src.patches_original[i]->buffer);
        src_patches[1] = &(src.patches_LPE1[i]->buffer);
        src_patches[2] = &(src.patches_LPE2[i]->buffer);
        src_patches[3] = &(src.patches_LPE3[i]->buffer);

        double energy = Energy(src_patches, tgt_patches, src_stylized_patch, tgt_style, 2.0);
        if (energy < min_energy) {
            nearest = xy - index_to_position(i, src.width);
            min_energy = energy;
        }
    }

    return nearest;
}

void Stylit::average(int index){
    output[index] = toRGBA(Vector3f(0, 0, 0));
}

