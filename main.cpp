#include "mainwindow.h"

#include <QApplication>
#include "util.h"
#include "stylit.h"

#include "Eigen/Dense"
using namespace Eigen;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load in sphere mesh + buffers
    auto tup = loadImageFromFile("Data/sphere.bmp");
    saveImageToFile("OUTPUT.png", std::get<0>(tup), std::get<1>(tup), std::get<2>(tup));

    // load in target mesh + buffers

    // load in stylized imagexw

    int source_width;
    int source_height;

    int target_width;
    int target_height;

    std::vector<RGBA> source_color_rbgs;
    std::vector<RGBA> source_LPE1_rbgs;
    std::vector<RGBA> source_LPE2_rbgs;
    std::vector<RGBA> source_LPE3_rbgs;

    std::vector<RGBA> target_color_rbgs;
    std::vector<RGBA> target_LPE1_rbgs;
    std::vector<RGBA> target_LPE2_rbgs;
    std::vector<RGBA> target_LPE3_rbgs;

    std::vector<RGBA> source_style_rbgs;

    Stylit stylit(source_width, source_height, target_width, target_height,
                  source_color_rbgs, source_LPE1_rbgs, source_LPE2_rbgs, source_LPE3_rbgs,
                  target_color_rbgs, target_LPE1_rbgs, target_LPE2_rbgs, target_LPE3_rbgs,
                  source_style_rbgs);

    stylit.run(6);

    MainWindow w;
    w.show();
    return a.exec();
}
