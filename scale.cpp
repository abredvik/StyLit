#include "scale.h"

//scale::scale()
//{

//}

auto& Scale::getPixelRepeated(auto& data, int width, int height, int x, int y) {
    int newX = (x < 0) ? 0 : std::min(x, width  - 1);
    int newY = (y < 0) ? 0 : std::min(y, height - 1);
    return data[width * newY + newX];
}

double Scale::triangle_filter(double x, double radius, float scale) {
    static double prev_radius, prev_c1, prev_c2;
    double x_fabs = std::fabs(x);
    radius = (scale < 1) ? std::max(2.0, radius) : radius;
    if (x_fabs > radius) {
        return 0;
    } else if (radius != prev_radius) {
        prev_radius = radius;
        prev_c1 = 1.0 / radius;
        prev_c2 = -prev_c1 / radius;

    }
    return prev_c1 + (prev_c2 * x_fabs);
}

RGBA Scale::back_map(std::vector<RGBA>& src, int width, int x, int y, float scale, bool x_dir) {
    float radius = (scale > 1) ? 1.0 : 1.0 / scale;
    float interp = (x_dir) ? x : y;
    interp /= scale;
    interp += (1 - scale) / scale / 2;

    int left = std::ceil(interp - radius);
    int right = std::floor(interp + radius);

    float red_acc, green_acc, blue_acc, weights_sum, weight;
    int xc, yc;
    RGBA pix;

    red_acc = green_acc = blue_acc = weights_sum = 0;
    for (int i = left; i <= right; ++i) {
        xc = (x_dir) ? i : x;
        yc = (x_dir) ? y : i;
        pix = getPixelRepeated(src, width, src.size() / width, xc, yc);
        weights_sum += weight;
        weight = triangle_filter(i - interp, radius, scale);
        red_acc += weight * (float)pix.r / 255.0;
        green_acc += weight * (float)pix.g / 255.0;
        blue_acc += weight * (float)pix.b / 255.0;
    }

    Vector3f res = Vector3f(0.f, 0.f, 0.f);
    res[0] = (scale > 1) ? red_acc : red_acc / weights_sum;
    res[1] = (scale > 1) ? green_acc : green_acc / weights_sum;
    res[2] = (scale > 1) ? blue_acc : blue_acc / weights_sum;

    std::uint8_t red = (std::uint8_t)(255.f * std::min(std::max(res[0],0.f), 1.f));
    std::uint8_t green = (std::uint8_t)(255.f * std::min(std::max(res[1],0.f), 1.f));
    std::uint8_t blue = (std::uint8_t)(255.f * std::min(std::max(res[2],0.f), 1.f));

    return RGBA{red, green, blue, 255};
}

std::vector<RGBA> Scale::handle_scale(std::vector<RGBA> &image, int width, int height, float scaleX, float scaleY) {
    int new_width = std::round((float)width * scaleX);
    std::vector<RGBA> resultX(new_width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            RGBA pix = back_map(image, width, x, y, scaleX, true);
            resultX[y * new_width + x] = pix;
        }
    }

    int new_height = std::round((float)height * scaleY);
    std::vector<RGBA> resultY(new_width * new_height);
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            resultY[y * new_width + x] = back_map(resultX, new_width, x, y, scaleY, false);
        }
    }
    return resultY;
}
