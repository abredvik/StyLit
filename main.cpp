#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>

#include "util.h"
#include "stylit.h"
#include "convolve.h"

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

    //one argument - run config file
    if (args.size() == 1) {

        std::vector<QString> srcPaths;
        srcPaths.reserve(6);

        std::vector<QString> tgtPaths;
        srcPaths.reserve(6);

        Image srcImg, tgtImg;

        // get settings file
        QSettings settings(args[0], QSettings::IniFormat);

        int num_iterations = settings.value("params/num_iterations").toInt();

        const QString srcFolder = settings.value("source/folderPath").toString();
        const QString tgtFolder = settings.value("target/folderPath").toString();
        const QString stylization = settings.value("style/filePath").toString();

        auto styleTup = loadImageFromFile(stylization);
        std::vector<RGBA> style_RGBA = *std::get<0>(styleTup);
        srcImg.width = std::get<1>(styleTup);
        srcImg.height = std::get<2>(styleTup);

        auto blackTup = loadImageFromFile(tgtFolder + "black_square.bmp");
        std::vector<RGBA> black_RGBA =  *std::get<0>(blackTup);
        tgtImg.width = std::get<1>(blackTup);
        tgtImg.height = std::get<2>(blackTup);


        srcPaths.push_back(srcFolder + "color.bmp");
        srcPaths.push_back(srcFolder + "LPE1.bmp");
        srcPaths.push_back(srcFolder + "LPE2.bmp");
        srcPaths.push_back(srcFolder + "LPE3.bmp");
        srcPaths.push_back(srcFolder + "LPE4.png");

        tgtPaths.push_back(tgtFolder + "color.bmp");
        tgtPaths.push_back(tgtFolder + "LPE1.bmp");
        tgtPaths.push_back(tgtFolder + "LPE2.bmp");
        tgtPaths.push_back(tgtFolder + "LPE3.bmp");
        tgtPaths.push_back(tgtFolder + "LPE4.png");

        init_image(srcPaths, style_RGBA, srcImg, num_iterations);
        init_image(tgtPaths, black_RGBA, tgtImg, num_iterations);

        int original_tgt_width = tgtImg.width;
        int original_tgt_height = tgtImg.height;

        Stylit stylit;
        std::vector<RGBA> final_image_RGBA = stylit.run(srcImg, tgtImg, num_iterations);
        //saveImageToFile("Output/RECONSTRUCTION.png", stylized_image_RGBA, original_tgt_width, original_tgt_height);

        // sharpening:

        // seperate input
        //    auto tup = loadImageFromFile("Output/May_3rd/Green_og.png");
        //    const std::vector<RGBA>& RGBimage = std::get<0>(tup);
        //    std::vector<RGBA> stylized_image_RGBA = RGBimage;

        // or just use output of stylit
        Convolve convolve;
        std::vector<float> blur_kernel =  {1.f/5.f, 1/5.f, 1/5.f, 1.f/5.f, 1/5.f};
        std::vector<RGBA> sharpened = convolve.sharpen(final_image_RGBA, original_tgt_width, original_tgt_height, blur_kernel);
        saveImageToFile("Output/RECONSTRUCTION.png", sharpened, original_tgt_width, original_tgt_height);


    } else if (args.size() == 0) { // if no config file - have user draw
        MainWindow w;
        w.show();
        a.exec();
    } else {
        std::cout << "Too many arguments. Please just either nothing or a path to a config file (.ini) as a command-line argument." << std::endl;
        a.exit(1);
        return 1;
    }

    return 0;
}
