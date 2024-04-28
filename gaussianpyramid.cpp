#include "gaussianpyramid.h"

std::vector<std::vector<RGBA>> Gaussianpyramid::create_pyrimid(int num_iterations, std::vector<RGBA> image, int width, int height){

std::vector<std::vector<RGBA>> result(num_iterations);

    result[num_iterations - 1] = image;


    for(int i = num_iterations - 2; i >= 0; --i){
        result[i] = scale.handle_scale(result[i + 1], width, height, 0.5, 0.5);
        width >>= 1;
        height >>= 1;
    }

    return result;
}
