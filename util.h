#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <QString>
#include <iostream>

#include "rgba.h"

#include "Eigen/Dense"
using namespace Eigen;

typedef std::unordered_map<int, Vector2i> NNF_t;

struct Patch {
    // vectors of length 75 for now
    VectorXf buffer;
    Vector2i coordinates;
    std::vector<int> neighbor_patches;
    bool is_matched;
};

struct Image {
    int width;
    int height;
    std::vector<Patch*> patches_orignal;
    std::vector<Patch*> patches_LPE1;
    std::vector<Patch*> patches_LPE2;
    std::vector<Patch*> patches_LPE3;
    std::vector<Patch*> patches_stylized;
};

std::tuple<std::vector<RGBA>, int, int> loadImageFromFile(const QString &file);

/**
 * @brief Saves the current canvas image to the specified file path.
 * @param file: file path to save image to
 * @return True if successfully saves image, False otherwise.
 */
bool saveImageToFile(const QString &file, const std::vector<RGBA>& data, int width, int height);

/**
 * @brief Canvas2D::pos_to_index returns an index based on the given x and y coordinates
 * @param x
 * @param y
 * @param width
 */
int pos_to_index(const Vector2i& xy, int width);

/**
 * @brief Canvas2D::pos_to_index returns an index based on the given x and y coordinates
 * @param x
 * @param y
 * @param width
 */
Vector2i index_to_position(int index, int width);


/**
 * @brief Canvas2D::int_to_float returns a float corresponding to the given uint8_t
 * @param interger
 */
float uint8_to_float(std::uint8_t byte);

/**
 * @brief Canvas2D::float_to_int returns a integer corresponding to the given float
 * @param f
 */
std::uint8_t float_to_int(float f);

RGBA toRGBA(const Eigen::Vector3f &color);

Vector2i nearest_neighbor();

#endif // UTIL_H
