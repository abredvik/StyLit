#include "mainwindow.h"

#include <QApplication>
//#include "util.h"
//#include "stylit.h"
#include "patchmatch.h"

//#include "Eigen/Dense"
//using namespace Eigen;

//std::tuple<std::vector<RGBA>, int, int> loadImageFromFile(const QString &file) {
//    QImage myImage;
//    if (!myImage.load(file)) {
//        throw std::runtime_error("Failed to load in image");
//    }
//    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
//    int width = myImage.width();
//    int height = myImage.height();
//    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

//    std::vector<RGBA> data;
//    data.reserve(width * height);
//    for (int i = 0; i < arr.size() / 4.f; i++){
//        data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
//    }

//    return std::make_tuple(data, width, height);
//}

///**
// * @brief Saves the current canvas image to the specified file path.
// * @param file: file path to save image to
// * @return True if successfully saves image, False otherwise.
// */
//bool saveImageToFile(const QString &file, const std::vector<RGBA>& data, int width, int height) {
//    QImage myImage = QImage(width, height, QImage::Format_RGBX8888);
//    for (int i = 0; i < data.size(); i++){
//        myImage.setPixelColor(i % width, i / width, QColor(data[i].r, data[i].g, data[i].b, data[i].a));
//    }
//    if (!myImage.save(file)) {
//        std::cerr<< "Failed to save image" <<std::endl;
//        return false;
//    }
//    return true;
//}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load in sphere mesh + buffers
//    auto srcTup = loadImageFromFile("Data/sphere.bmp");
//    saveImageToFile("OUTPUT.png", std::get<0>(tup), std::get<1>(tup), std::get<2>(tup));

    // load in target mesh + buffers

    // load in stylized imagexw

    auto srcTup = loadImageFromFile("Data/test_1.png");
    const std::vector<RGBA>& source_image = std::get<0>(srcTup);
    int source_width = std::get<1>(srcTup);
    int source_height = std::get<2>(srcTup);
    std::vector<VectorXf> source_patches = get_patches(source_image, source_width, source_height);

    auto tgtTup = loadImageFromFile("Data/test_2.png");
    const std::vector<RGBA>& target_image = std::get<0>(tgtTup);
    int target_width = std::get<1>(tgtTup);
    int target_height = std::get<2>(tgtTup);
    std::vector<VectorXf> target_patches = get_patches(target_image, target_width, target_height);

    NNF_t NNF = patch_match(source_patches, target_patches, source_width, source_height);

    std::vector<RGBA> reconstruction = recreate_image(NNF, target_image, target_width);

    saveImageToFile("RECONSTRUCTION.png", reconstruction, target_width, target_height);

//    int target_width;
//    int target_height;

//    std::vector<RGBA> source_color_rbgs;
//    std::vector<RGBA> source_LPE1_rbgs;
//    std::vector<RGBA> source_LPE2_rbgs;
//    std::vector<RGBA> source_LPE3_rbgs;

//    std::vector<RGBA> target_color_rbgs;
//    std::vector<RGBA> target_LPE1_rbgs;
//    std::vector<RGBA> target_LPE2_rbgs;
//    std::vector<RGBA> target_LPE3_rbgs;

//    std::vector<RGBA> source_style_rbgs;

//    Stylit stylit(source_width, source_height, target_width, target_height,
//                  source_color_rbgs, source_LPE1_rbgs, source_LPE2_rbgs, source_LPE3_rbgs,
//                  target_color_rbgs, target_LPE1_rbgs, target_LPE2_rbgs, target_LPE3_rbgs,
//                  source_style_rbgs);

//    stylit.run(6);

    MainWindow w;
    w.show();
    return a.exec();
}
