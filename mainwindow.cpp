
#include "mainwindow.h"
#include "settings.h"
#include "stylit.h"
#include "convolve.h"
#include "scale.h"
#include "util.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QCheckBox>
#include <iostream>
#include <QButtonGroup>

QButtonGroup *brushButtons = new QButtonGroup();
QButtonGroup *meshButtons = new QButtonGroup();

MainWindow::MainWindow()
{
    setWindowTitle("2D Projects: Brush and Filter");

    // loads in settings from last run or uses default values
    settings.loadSettingsOrDefaults();

    QHBoxLayout *hLayout = new QHBoxLayout(); // horizontal layout for canvas and controls panel
    QHBoxLayout *hLayout_output = new QHBoxLayout(); // horizontal layout for canvas and controls panel
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical layout for control panel
    QVBoxLayout *vLayout_output = new QVBoxLayout(); // vertical layout for control panel

    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);

    //vLayout_output->setAlignment(Qt::AlignBottom);
    //hLayout_output->addLayout(vLayout_output);

    setLayout(hLayout);
    //setLayout(hLayout_output);

    setupCanvas2D();
    resize(800, 600);

    // makes the canvas into a scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(m_canvas);
    scrollArea->setWidgetResizable(true);
    hLayout->addWidget(scrollArea, 1);


    // groupings by project
    QWidget *brushGroup = new QWidget();
    QVBoxLayout *brushLayout = new QVBoxLayout();
    brushLayout->setAlignment(Qt::AlignTop);
    brushGroup->setLayout(brushLayout);

    QScrollArea *controlsScroll = new QScrollArea();
    QTabWidget *tabs = new QTabWidget();
    controlsScroll->setWidget(tabs);
    controlsScroll->setWidgetResizable(true);


    tabs->addTab(brushGroup, "Brush");

    vLayout->addWidget(controlsScroll);

    // brush selection
    addHeading(brushLayout, "Brush");
    addRadioButton(brushLayout, "Constant", settings.brushType == BRUSH_CONSTANT, [this]{ setBrushType(BRUSH_CONSTANT); }, true);
    addRadioButton(brushLayout, "Linear", settings.brushType == BRUSH_LINEAR, [this]{ setBrushType(BRUSH_LINEAR); }, true);
    addRadioButton(brushLayout, "Quadratic", settings.brushType == BRUSH_QUADRATIC, [this]{ setBrushType(BRUSH_QUADRATIC); }, true);
    addRadioButton(brushLayout, "Smudge", settings.brushType == BRUSH_SMUDGE, [this]{ setBrushType(BRUSH_SMUDGE); }, true);

    // brush parameters
    addSpinBox(brushLayout, "red", 0, 255, 1, settings.brushColor.r, [this](int value){ setUIntVal(settings.brushColor.r, value); });
    addSpinBox(brushLayout, "green", 0, 255, 1, settings.brushColor.g, [this](int value){ setUIntVal(settings.brushColor.g, value); });
    addSpinBox(brushLayout, "blue", 0, 255, 1, settings.brushColor.b, [this](int value){ setUIntVal(settings.brushColor.b, value); });
    addSpinBox(brushLayout, "alpha", 0, 255, 1, settings.brushColor.a, [this](int value){ setUIntVal(settings.brushColor.a, value); });
    addSpinBox(brushLayout, "radius", 0, 100, 1, settings.brushRadius, [this](int value){ setIntVal(settings.brushRadius, value); });

    // clearing canvas
    addPushButton(brushLayout, "Clear canvas", &MainWindow::onClearButtonClick);

    //upload image
    addPushButton(brushLayout, "Upload Image", &MainWindow::onUploadButtonClick);

    // save canvas as image
    addPushButton(brushLayout, "Save Image", &MainWindow::onSaveButtonClick);

    // Target Mesh selection
    addHeading(brushLayout, "Target Mesh");
    addRadioButton(brushLayout, "Guy", settings.targetMeshType == MESH_GUY, [this]{ setTargetMeshType(MESH_GUY); }, false);
    addRadioButton(brushLayout, "Tea Pot", settings.targetMeshType == MESH_TEAPOT, [this]{ setTargetMeshType(MESH_TEAPOT); }, false);
    addRadioButton(brushLayout, "Armadillo", settings.targetMeshType == MESH_ARMADILLO, [this]{ setTargetMeshType(MESH_ARMADILLO); }, false);
    addRadioButton(brushLayout, "Bunny", settings.targetMeshType == MESH_BUNNY, [this]{ setTargetMeshType(MESH_BUNNY); }, false);
    addRadioButton(brushLayout, "Bell Pepper", settings.targetMeshType == MESH_BELLPEPPER, [this]{ setTargetMeshType(MESH_BELLPEPPER); }, false);

    // stylize with image
    addPushButton(brushLayout, "Stylize Drawing", &MainWindow::onStylizeButtonClick);

}

/**
 * @brief Sets up Canvas2D
 */
void MainWindow::setupCanvas2D() {
    m_canvas = new Canvas2D();
    m_canvas->init();

    if (!settings.imagePath.isEmpty()) {
        m_canvas->loadImage(settings.imagePath);
    }
}


// ------ FUNCTIONS FOR ADDING UI COMPONENTS ------

void MainWindow::addHeading(QBoxLayout *layout, QString text) {
    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    QLabel *label = new QLabel(text);
    label->setFont(font);
    layout->addWidget(label);
}

void MainWindow::addLabel(QBoxLayout *layout, QString text) {
    layout->addWidget(new QLabel(text));
}

void MainWindow::addRadioButton(QBoxLayout *layout, QString text, bool value, auto function, bool brush) {
    QRadioButton *button = new QRadioButton(text);
    button->setChecked(value);
    layout->addWidget(button);
    connect(button, &QRadioButton::clicked, this, function);
    if(brush){
        brushButtons->addButton(button);
    } else {
        meshButtons->addButton(button);
    }
}

void MainWindow::addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function) {
    QSpinBox *box = new QSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, function);
}

void MainWindow::addDoubleSpinBox(QBoxLayout *layout, QString text, double min, double max, double step, double val, int decimal, auto function) {
    QDoubleSpinBox *box = new QDoubleSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    box->setDecimals(decimal);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, function);
}

void MainWindow::addPushButton(QBoxLayout *layout, QString text, auto function) {
    QPushButton *button = new QPushButton(text);
    layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, function);
}

void MainWindow::addCheckBox(QBoxLayout *layout, QString text, bool val, auto function) {
    QCheckBox *box = new QCheckBox(text);
    box->setChecked(val);
    layout->addWidget(box);
    connect(box, &QCheckBox::clicked, this, function);
}



// ------ FUNCTIONS FOR UPDATING SETTINGS ------

void MainWindow::setBrushType(int type) {
    settings.brushType = type;
    m_canvas->settingsChanged();
}

void MainWindow::setTargetMeshType(int type) {
    settings.targetMeshType = type;
    m_canvas->settingsChanged();
}

void MainWindow::setUIntVal(std::uint8_t &setValue, int newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setIntVal(int &setValue, int newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setFloatVal(float &setValue, float newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setBoolVal(bool &setValue, bool newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}


// ------ PUSH BUTTON FUNCTIONS ------

void MainWindow::onClearButtonClick() {
    //m_canvas->resize(m_canvas->parentWidget()->size().width(), m_canvas->parentWidget()->size().height());
    m_canvas->clearCanvas();
}

void MainWindow::onRevertButtonClick() {
    m_canvas->loadImage(settings.imagePath);
}

void MainWindow::onUploadButtonClick() {
    // Get new image path selected by user
    QString file = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) { return; }
    settings.imagePath = file;

    // Display new image
    m_canvas->loadImage(settings.imagePath);

    m_canvas->settingsChanged();
}

void MainWindow::onSaveButtonClick() {
    // Get new image path selected by user
    QString file = QFileDialog::getSaveFileName(this, tr("Save Image"), QDir::currentPath(), tr("Image Files (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) { return; }

    // Save image
    m_canvas->saveImageToFile(file);
}

void MainWindow::onTargetMeshButtonClick() {

}

void MainWindow::onStylizeButtonClick() {

    //scale down to 128
    Scale scale;
    m_canvas->m_data =  scale.handle_scale(m_canvas->m_data, m_canvas->m_width, m_canvas->m_height, 0.25f, 0.25f);
    m_canvas->m_width = 128;
    m_canvas->m_height = 128;

    int num_iterations = 6;

    std::vector<QString> srcPaths;
    srcPaths.reserve(5);

    std::vector<QString> tgtPaths;
    srcPaths.reserve(5);

    const QString srcFolder = "Data/Sphere_128/";
    QString tgtFolder; // TO DO: user selection

    Image srcImg, tgtImg;
    tgtImg.width = 128;
    tgtImg.height = 128;
    srcImg.width = 128;
    srcImg.height = 128;

    switch(settings.targetMeshType) {
    case MESH_GUY:
        tgtFolder = "Data/Guy_128/";
        break;
    case MESH_TEAPOT:
        tgtFolder = "Data/Teapot_128/";
        break;
    case MESH_ARMADILLO:
        tgtFolder = "Data/Armadillo_128/";
        break;
    case MESH_BUNNY:
        tgtFolder = "Data/Bunny_128/";
        break;
    case MESH_BELLPEPPER:
        tgtFolder = "Data/Bell_Pepper_128/";
        break;
    }

    auto blackTup = loadImageFromFile(tgtFolder + "black_square.bmp");
    std::vector<RGBA> black_RGBA =  *std::get<0>(blackTup);

    srcPaths.push_back(srcFolder + "color.bmp");
    srcPaths.push_back(srcFolder + "LPE1.bmp");
    srcPaths.push_back(srcFolder + "LPE2.bmp");
    srcPaths.push_back(srcFolder + "LPE3.bmp");
    srcPaths.push_back(srcFolder + "LPE4.bmp");

    tgtPaths.push_back(tgtFolder + "color.bmp");
    tgtPaths.push_back(tgtFolder + "LPE1.bmp");
    tgtPaths.push_back(tgtFolder + "LPE2.bmp");
    tgtPaths.push_back(tgtFolder + "LPE3.bmp");
    tgtPaths.push_back(tgtFolder + "LPE4.bmp");

    init_image(srcPaths, m_canvas->m_data, srcImg, num_iterations);
    init_image(tgtPaths, black_RGBA, tgtImg, num_iterations);

    Stylit stylit;
    std::vector<RGBA> stylized_image_RGBA = stylit.run(srcImg, tgtImg, num_iterations);

    //scale back up
    std::vector<RGBA> final_image =  scale.handle_scale(stylized_image_RGBA, m_canvas->m_width, m_canvas->m_height, 4.f, 4.f);

    //sharpen
    Convolve convolve;
    final_image = convolve.sharpen(final_image, 512, 512);

    saveImageToFile("Output/interactive.png", final_image, 512, 512);

    // save image
    m_canvas->m_width = 512;
    m_canvas->m_height = 512;
    m_canvas->m_data = final_image;
    m_canvas->displayImage();
}
