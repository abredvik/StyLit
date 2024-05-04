#include "convolve.h"

//Convolve::Convolve()
//{

//}

//void Convolve::handle_blur(std::vector<Vector3f> &image, int width) {
//    std::vector<float> kernel = gaussian_kernel(3);
//    std::vector<Vector3f> floatData(image.size());
//    for (int i = 0; i < image.size(); ++i) {
//        floatData[i] = image[i];
//    }
//    std::vector<Vector3f> colResult = convolve(floatData, width, kernel, false);
//    std::vector<Vector3f> rowResult = convolve(colResult, width, kernel, true);
//    for (int i = 0; i < image.size(); ++i) {
//        image[i] = rowResult[i];
//    }
//}

auto& Convolve::getPixelRepeated(auto& data, int width, int height, int x, int y) {
    int newX = (x < 0) ? 0 : std::min(x, width  - 1);
    int newY = (y < 0) ? 0 : std::min(y, height - 1);
    return data[width * newY + newX];
}

std::vector<Vector3f> Convolve::convolve(std::vector<Vector3f>& img, int width, const std::vector<float>& kernel, bool x_dir) {
    int height = img.size() / width;
    int radius = kernel.size() / 2;
    float red_acc, green_acc, blue_acc;
    int center_index;
    int xc, yc;
    Vector3f* pix;
    std::vector<Vector3f> result(img.size());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            center_index = y * width + x;
            Vector3f accumulator = Vector3f(0.f, 0.f, 0.f);
            for (int i = 0; i <= 2 * radius; ++i) {
                xc = (!x_dir) ? x : x + i - radius;
                yc = (x_dir) ? y : y + i - radius;
                pix = &getPixelRepeated(img, width, height, xc, yc);
                accumulator += kernel[i] * (*pix);
            }
            result[center_index] = accumulator;
        }
    }

    return result;
}

float Convolve::gaussian_func(float x) {
    static const float c1 = 3.0 / std::sqrt(2.0 * std::numbers::pi);
    static const float c2 = -9.0 / 2.0;
    float r = (float)3;
    return (r == 0) ? 1.0 : c1 / r * std::exp(c2 * x * x / r / r);
}

std::vector<float> Convolve::gaussian_kernel(int radius) {
    // fill vector
    std::vector<float> res;
    float val, sum = 0;
    for (int i = -radius; i <= radius; ++i) {
        val = gaussian_func((float)i);
        sum += val;
        res.push_back(val);
    }

    // normalize kernel
    for (int i = 0; i < res.size(); ++i) {
        res[i] /= sum;
    }

    return res;
}

std::vector<RGBA> Convolve::sharpen(std::vector<RGBA> stylized_image_RGBA, int width, int height){

    //convert to floats
    std::vector<Vector3f> stylized_image_float(stylized_image_RGBA.size());
    for(int i = 0; i < stylized_image_RGBA.size(); i++){
        stylized_image_float[i] = Vector3f(uint8_to_float(stylized_image_RGBA[i].r), uint8_to_float(stylized_image_RGBA[i].g), uint8_to_float(stylized_image_RGBA[i].b));
    }

    // blur image
    Convolve convolve;
    std::vector<float> blur_kernel = gaussian_kernel(5);
    std::vector<Vector3f> blurred_image = convolve.convolve(stylized_image_float, width, blur_kernel, true);
    blurred_image = convolve.convolve(blurred_image, height, blur_kernel, false);

    std::vector<RGBA> sharpened(stylized_image_RGBA.size());
    for(int i = 0; i < sharpened.size(); i++){
        Vector3f temp = stylized_image_float[i] + (stylized_image_float[i] - blurred_image[i]);
        sharpened[i] = toRGBA(temp);
    }

    return sharpened;
}
