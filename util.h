#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <QString>
#include <iostream>

#include "rgba.h"

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

#endif // UTIL_H
