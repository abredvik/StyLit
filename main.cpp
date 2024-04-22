#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>

#include "util.h"
#include "stylit.h"
#include "patchmatch.h"

#include "Eigen/Dense"
using namespace Eigen;

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
//        printf("Not enough arguments. Please provide a path to a config file (.ini) as a command-line argument.\n");
        std::cout << "Not enough arguments. Please provide a path to a config file (.ini) as a command-line argument." << std::endl;
        a.exit(1);
        return 1;
    }

    // get settings file
    QSettings settings(args[0], QSettings::IniFormat);

    const QString srcFolder = settings.value("source/folderPath").toString();
    const QString tgtFolder = settings.value("target/folderPath").toString();
    const QString stylization = settings.value("style/filePath").toString();

    std::vector<QString> srcPaths;
    srcPaths.reserve(5);
    srcPaths.push_back(srcFolder + "/color.bmp");
    srcPaths.push_back(srcFolder + "/LPE1.bmp");
    srcPaths.push_back(srcFolder + "/LPE2.bmp");
    srcPaths.push_back(srcFolder + "/LPE3.bmp");
    srcPaths.push_back(stylization);

    std::vector<QString> tgtPaths;
    srcPaths.reserve(5);
    tgtPaths.push_back(tgtFolder + "/color.bmp");
    tgtPaths.push_back(tgtFolder + "/LPE1.bmp");
    tgtPaths.push_back(tgtFolder + "/LPE2.bmp");
    tgtPaths.push_back(tgtFolder + "/LPE3.bmp");
    tgtPaths.push_back(tgtFolder + "/color.bmp");

    Image srcImg, tgtImg;
    init_image(srcPaths, srcImg);
    init_image(tgtPaths, tgtImg);

    Patchmatcher patchMatcher;

    NNF_t NNF = patchMatcher.patch_match(srcImg, tgtImg);

    std::vector<RGBA> reconstruction = recreate_image(NNF, tgtImg);

    saveImageToFile("Output/RECONSTRUCTION.png", reconstruction, tgtImg.width, tgtImg.height);

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
