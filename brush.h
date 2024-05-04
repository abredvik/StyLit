/**
 * @file    Brush.h
 *
 */

#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QObject>
#include "rgba.h"
#include <array>
//#include "settings.h"

/**
 * @struct Brush
 *
*/
struct Brush {
    std::vector<float> mask;
    void create_mask(int type_of_mask);
    int radius;
    RGBA color;
    std::vector<RGBA> picked_up;

    void create_constant_mask(int type_of_brush, int size_mask, int radius);
    void create_linear_mask(int type_of_brush, int size_mask, int radius);
    void create_quadratic_mask(int type_of_brush, int size_mask, int radius);
    //void create_spray_mask(int type_of_brush, int size_of_mask, int radius);
};




// The global Settings object, will be initialized by MainWindow
extern Brush brush;

#endif // BRUSH_H
