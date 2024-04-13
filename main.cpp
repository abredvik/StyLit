#include "mainwindow.h"

#include <QApplication>
#include "util.h"

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

    MainWindow w;
    w.show();
    return a.exec();
}
