#include "stylit.h"
#include "patchmatch.h"

Stylit::Stylit(int source_width, int source_height, int target_width, int target_height,
               std::vector<RGBA> source_color_rbgs, std::vector<RGBA> source_LPE1_rbgs, std::vector<RGBA> source_LPE2_rbgs, std::vector<RGBA> source_LPE3_rbgs,
               std::vector<RGBA> target_color_rbgs, std::vector<RGBA> target_LPE1_rbgs, std::vector<RGBA> target_LPE2_rbgs, std::vector<RGBA> target_LPE3_rbgs,
               std::vector<RGBA> source_style_rbgs)
{

    //TO DO: initialize stuff

}

std::vector<RGBA> Stylit::run(int iterations){
    for(int i = 0; i < iterations; i++){
        stylit_algorithm();
    }
    return output;
}

void Stylit::stylit_algorithm(){

    Patchmatcher patchmatcher;

    int targets_covered = 0;
    int total_targets = target_image.patches_orignal.size();
    float percent_coverage = 0.f;

    while(percent_coverage < 0.80f){
        // TO DO: Write patch_match()
        NNF_t temp_NNF = patchmatcher.patch_match();
        std::pair<int, float> error_budget_pair= calculate_error_budget();
        int k = error_budget_pair.first;
        float T = error_budget_pair.second;

        for (int i = 0; i < k; i ++){
            int error_index = patchmatcher.errors[i].first;

            if(target_image.patches_orignal[error_index]->is_matched){
                continue;
            }

            targets_covered ++;
            target_image.patches_orignal[error_index]->is_matched = true;
            target_image.patches_LPE1[error_index]->is_matched = true;
            target_image.patches_LPE2[error_index]->is_matched = true;
            target_image.patches_LPE3[error_index]->is_matched = true;
            final_NNF[error_index] = temp_NNF[error_index];

        }

        percent_coverage = (float) targets_covered / (float) total_targets;
    }

    fill_gaps();

    for(int i = 0; i < (target_image.width * target_image.height); i ++){
        // TO DO: Write average()
        average(i);
    }

}

std::pair<int, float> Stylit::calculate_error_budget(){
    return std::make_pair(0, 0);
}

void Stylit::fill_gaps(){
    for(int i = 0; i < target_image.patches_orignal.size(); i++){
        if(!target_image.patches_orignal[i]->is_matched){
            // TO DO: write nearest_neighbor()
            final_NNF[i] = nearest_neighbor();
        }
    }
}

void Stylit::average(int index){
    output[index] = toRGBA(Vector3f(0, 0, 0));
}

