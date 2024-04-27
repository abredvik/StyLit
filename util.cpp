#include "util.h"
#include <random>

std::tuple<std::vector<RGBA>, int, int> loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        throw std::runtime_error("Failed to load in image");
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    int width = myImage.width();
    int height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    std::vector<RGBA> data;
    data.reserve(width * height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }

    return std::make_tuple(data, width, height);
}

/**
 * @brief Saves the current canvas image to the specified file path.
 * @param file: file path to save image to
 * @return True if successfully saves image, False otherwise.
 */
bool saveImageToFile(const QString &file, const std::vector<RGBA>& data, int width, int height) {
    QImage myImage = QImage(width, height, QImage::Format_RGBX8888);
    for (int i = 0; i < data.size(); i++){
        myImage.setPixelColor(i % width, i / width, QColor(data[i].r, data[i].g, data[i].b, data[i].a));
    }
    if (!myImage.save(file)) {
        std::cerr<< "Failed to save image" <<std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Canvas2D::pos_to_index returns an index based on the given x and y coordinates
 * @param x
 * @param y
 * @param width
 */
int pos_to_index(const Vector2i& xy, int width) {
    int index = (xy[1] * width) + xy[0];
    return index;
}

/**
 * @brief Canvas2D::pos_to_index returns an index based on the given x and y coordinates
 * @param x
 * @param y
 * @param width
 */
Vector2i index_to_position(int index, int width) {
    Vector2i coordinates;
    coordinates[0] = index % width;
    coordinates[1] = index / width;
    return coordinates;
}


/**
 * @brief Canvas2D::int_to_float returns a float corresponding to the given uint8_t
 * @param interger
 */
float uint8_to_float(std::uint8_t byte) {
    return byte / float(255);
}

/**
 * @brief Canvas2D::float_to_int returns a integer corresponding to the given float
 * @param f
 */
std::uint8_t float_to_int(float f) {
    return round(255 * f);
}

RGBA toRGBA(const Eigen::Vector3f &color) {

    std::uint8_t red = (std::uint8_t)(255.f * std::min(std::max(color[0],0.f), 1.f));
    std::uint8_t green = (std::uint8_t)(255.f * std::min(std::max(color[1],0.f), 1.f));
    std::uint8_t blue = (std::uint8_t)(255.f * std::min(std::max(color[2],0.f), 1.f));

    return RGBA{red, green, blue, 255};
}

Vector2i nearest_neighbor(){
    return Vector2i(0, 0);
}

std::vector<RGBA> random_pixels(std::vector<RGBA>& RGBimage){
    std::random_device                      rand_dev;
    std::mt19937                            generator(rand_dev());
    std::uniform_real_distribution<double>   distr(-1.0, 1.0);
    std::shuffle(RGBimage.begin(), RGBimage.end(), generator);
    return RGBimage;
}
