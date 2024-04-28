#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>

#include "util.h"
#include "stylit.h"

#include "Eigen/Dense"
using namespace Eigen;

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // create parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("config",  "Path of the config (.ini) file.");
    parser.process(a);

    // Check for invalid argument count
    const QStringList args = parser.positionalArguments();
    if (args.size() < 1) {
        std::cout << "Not enough arguments. Please provide a path to a config file (.ini) as a command-line argument." << std::endl;
        a.exit(1);
        return 1;
    }

    // get settings file
    QSettings settings(args[0], QSettings::IniFormat);

    const QString srcFolder = settings.value("source/folderPath").toString();
    const QString tgtFolder = settings.value("target/folderPath").toString();
    const QString stylization = settings.value("style/filePath").toString();
    const int num_iterations = settings.value("params/num_iterations").toInt();

    std::vector<QString> srcPaths;

    srcPaths.reserve(6);
    srcPaths.push_back(srcFolder + "color.bmp");
    srcPaths.push_back(srcFolder + "LPE1.bmp");
    srcPaths.push_back(srcFolder + "LPE2.bmp");
    srcPaths.push_back(srcFolder + "LPE3.bmp");
    srcPaths.push_back(srcFolder + "LPE4.png");
    srcPaths.push_back(stylization);

    std::vector<QString> tgtPaths;
    srcPaths.reserve(6);
    tgtPaths.push_back(tgtFolder + "color.bmp");
    tgtPaths.push_back(tgtFolder + "LPE1.bmp");
    tgtPaths.push_back(tgtFolder + "LPE2.bmp");
    tgtPaths.push_back(tgtFolder + "LPE3.bmp");
    tgtPaths.push_back(tgtFolder + "LPE4.png");
    tgtPaths.push_back(tgtFolder + "black_square.bmp");

    Image srcImg, tgtImg;
    init_image(srcPaths, srcImg, num_iterations);
    init_image(tgtPaths, tgtImg, num_iterations);

    int original_tgt_width = tgtImg.width;
    int original_tgt_height = tgtImg.height;

    Stylit stylit;

//    const QString& filename  = settings.value("style/filePath").toString();
//    auto tup = loadImageFromFile(filename);
//    std::vector<RGBA>& RGBimage = std::get<0>(tup);
//    saveImageToFile("Output/Random.png", random_pixels(RGBimage), 256, 256);
//    return 1;

    saveImageToFile("Output/RECONSTRUCTION.png", stylit.run(srcImg, tgtImg, num_iterations), original_tgt_width, original_tgt_height);

    MainWindow w;
    w.show();
    return a.exec();
}
