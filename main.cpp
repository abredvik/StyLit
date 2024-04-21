#include "mainwindow.h"

#include <QApplication>
#include "util.h"
#include "stylit.h"
#include "patchmatch.h"

#include "Eigen/Dense"
using namespace Eigen;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load in sphere mesh + buffers
//    auto srcTup = loadImageFromFile("Data/sphere.bmp");
//    saveImageToFile("OUTPUT.png", std::get<0>(tup), std::get<1>(tup), std::get<2>(tup));

    // load in target mesh + buffers

    // load in stylized imagexw

    Image srcImg;

    auto srcTup = loadImageFromFile("Data/test_1.png");
    const std::vector<RGBA>& source_image = std::get<0>(srcTup);
    int source_width = std::get<1>(srcTup);
    int source_height = std::get<2>(srcTup);
    srcImg.height = source_height;
    srcImg.width = source_width;

//    std::vector<VectorXf> source_patches = get_patches(source_image, source_width, source_height);

    auto tgtTup = loadImageFromFile("Data/test_2.png");
    const std::vector<RGBA>& target_image = std::get<0>(tgtTup);
    int target_width = std::get<1>(tgtTup);
    int target_height = std::get<2>(tgtTup);
//    std::vector<VectorXf> target_patches = get_patches(target_image, target_width, target_height);

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
