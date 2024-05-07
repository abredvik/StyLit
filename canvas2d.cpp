#include "canvas2d.h"
#include <tuple>
#include "util.h"

Common common;

/**
 * @brief Initializes new 500x500 canvas
 */
void Canvas2D::init() {
    setMouseTracking(true);
    m_width = 512;
    m_height = 512;
    clearCanvas();
    brush.radius = settings.brushRadius;
    brush.create_mask(settings.brushType);
    common.set(m_width, m_height);
}

/**
 * @brief Canvas2D::clearCanvas sets all canvas pixels to blank white
 */
void Canvas2D::clearCanvas() {
    //m_data.assign(m_width * m_height, RGBA{255, 255, 255, 255});
    std::tuple<std::vector<RGBA>*, int, int> stenciltup = loadImageFromFile("Data/stencil_zoomed.png");
    std::vector<RGBA> stencil_RGBA =  *std::get<0>(stenciltup);
    m_data = stencil_RGBA;
    settings.imagePath = "";
    displayImage();
}

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * Also saves the image width and height to canvas width and height respectively.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
bool Canvas2D::loadImage(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        m_data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    displayImage();
    return true;
}

/**
 * @brief Saves the current canvas image to the specified file path.
 * @param file: file path to save image to
 * @return True if successfully saves image, False otherwise.
 */
bool Canvas2D::saveImageToFile(const QString &file) {
    QImage myImage = QImage(m_width, m_height, QImage::Format_RGBX8888);
    for (int i = 0; i < m_data.size(); i++){
        myImage.setPixelColor(i % m_width, i / m_width, QColor(m_data[i].r, m_data[i].g, m_data[i].b, m_data[i].a));
    }
    if (!myImage.save(file)) {
        std::cout<<"Failed to save image"<<std::endl;
        return false;
    }
    return true;
}


/**
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::displayImage() {
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(m_data.data()), 4*m_data.size());
    QImage now = QImage((const uchar*)img->data(), m_width, m_height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(now));
    setFixedSize(m_width, m_height);
    update();
}

/**
 * @brief Canvas2D::resize resizes canvas to new width and height
 * @param w
 * @param h
 */
void Canvas2D::resize(int w, int h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h);
    displayImage();
}

/**
 * @brief Called when any of the parameters in the UI are modified.
 */
void Canvas2D::settingsChanged() {
    // this saves your UI settings locally to load next time you run the program
    settings.saveSettings();

    brush.radius = settings.brushRadius;
    brush.create_mask(settings.brushType);

}

/**
 * @brief These (next 3) functions are called when the mouse is clicked and dragged on the canvas
 */
void Canvas2D::mouseDown(int x, int y) {
    if (settings.brushType == 3){
        int size = ((2 * brush.radius) + 1) * ((2 * brush.radius) + 1);
        brush.picked_up.resize(size);
        pick_up(x, y);
    }

    apply_brush(x, y, x - brush.radius, x + brush.radius, y-brush.radius, y + brush.radius);

    m_isDown = true;
    displayImage();
}

void Canvas2D::mouseDragged(int x, int y) {
    if (m_isDown){
        apply_brush(x, y, x - brush.radius, x + brush.radius, y - brush.radius, y + brush.radius);
        if (settings.brushType == 3){
            pick_up(x, y);
        }
        displayImage();
    }
}

void Canvas2D::mouseUp(int x, int y) {
    m_isDown = false;
}

/**
 * @brief Canvas2D:: apply_brush alters the data of the canvas to match what the selected brush should be "placing" down
 * @param x
 * @param y
 * @param x_min
 * @param x_max
 * @param y_min
 * @param y_max
 */
void Canvas2D::apply_brush(int x, int y, int x_min, int x_max, int y_min, int y_max) {
    int mask_x = 0;
    int mask_y = 0;

    for (int i = x - brush.radius; i <= x + brush.radius; i++){
        for (int j = y - brush.radius; j <= y + brush.radius; j++){
            if (common.is_in_range(i, j)){
                //only apply the brush if the pixel we are at is within the canvas
                int index = common.pos_to_index(i, j, m_width);
                int mask_index = common.pos_to_index(mask_x, mask_y, (2*settings.brushRadius) + 1);
                float opacity = brush.mask.at(mask_index);
                RGBA color = settings.brushColor;
                float alph = common.int_to_float(color.a);
                float new_red;
                float new_green;
                float new_blue;
                if (settings.brushType == 3){
                    //if we are using the smudge brush
                    float picked_up_r = common.int_to_float(brush.picked_up.at(mask_index).r);
                    float picked_up_g = common.int_to_float(brush.picked_up.at(mask_index).g);
                    float picked_up_b = common.int_to_float(brush.picked_up.at(mask_index).b);
                    new_red = ((opacity) * picked_up_r) + ((1 - (opacity)) * common.int_to_float(m_data.at(index).r));
                    new_green = ((opacity) * picked_up_g) + ((1 - (opacity)) * common.int_to_float(m_data.at(index).g));
                    new_blue = ((opacity) * picked_up_b) + ((1 - (opacity)) * common.int_to_float(m_data.at(index).b));
                    m_data.at(index) = RGBA{common.float_to_int(new_red), common.float_to_int(new_green),
                                            common.float_to_int(new_blue), common.float_to_int(alph)};
                } else {
                    new_red = ((opacity * alph) * common.int_to_float(color.r)) +
                              ((1 - (opacity * alph)) * common.int_to_float(m_data.at(index).r));
                    new_green = ((opacity * alph) * common.int_to_float(color.g)) +
                                ((1 - (opacity * alph)) * common.int_to_float(m_data.at(index).g));
                    new_blue = ((opacity * alph) * common.int_to_float(color.b)) +
                               ((1 - (opacity * alph)) * common.int_to_float(m_data.at(index).b));
                    m_data.at(index) = RGBA{common.float_to_int(new_red), common.float_to_int(new_green),
                                            common.float_to_int(new_blue), m_data.at(index).a};
                }
            }
            mask_y = mask_y + 1;
        }
        mask_y = 0;
        mask_x = mask_x + 1;
    }
}

/**
 * @brief Canvas2D::pick_up fills the picked_up field of a brush with the colors already on the canvas
 *                          at the place corresponding to coordinates (x, y)
 * @param x
 * @param y
 */
void Canvas2D::pick_up(int x, int y){
    int mask_x = 0;
    int mask_y = 0;
    for (int i = x - brush.radius; i <= x + brush.radius; i++){
        for (int j = y - brush.radius; j <= y + brush.radius; j++){
            if(common.is_in_range(i, j)){
                int index = common.pos_to_index(i, j, m_width);
                int m_index = common.pos_to_index(mask_x, mask_y, (2*brush.radius) + 1);
                brush.picked_up.at(m_index) = m_data.at(index);
                mask_y = mask_y + 1;
            }
        }
        mask_y = 0;
        mask_x = mask_x + 1;
    }
}
