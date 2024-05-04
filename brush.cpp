#include "brush.h"
#include <cmath>

Brush brush;
void Brush::create_mask(int type_of_brush){

    int size_of_mask = ((2 * radius) + 1) * ((2 * radius) + 1);
    mask.resize(size_of_mask);

    if (radius == 0){
        mask.at(0) = 1;
    } else if (type_of_brush == 0){
        //constant
        create_constant_mask(type_of_brush, size_of_mask, radius);

    } else if (type_of_brush == 1){
        //linear
        create_linear_mask(type_of_brush, size_of_mask, radius);

        //    } else if (type_of_brush == 4){
        //        //spray (EXTRA CREDIT)
        //        create_spray_mask(type_of_brush, size_of_mask, radius);
    } else {
        //quadratic (AND FOR SMUDGE)
        create_quadratic_mask(type_of_brush, size_of_mask, radius);
    }
}

void Brush::create_constant_mask(int type_of_brush, int size_of_mask, int radius){
    for (int i = 0; i < size_of_mask; i ++){
        int distance;
        distance = sqrt(((round(i/(((2 * radius) + 1))) - radius) * (round(i/(((2 * radius) + 1))) - radius))
                        +  ((i%(((2 * radius) + 1)) - radius) * (i%(((2 * radius) + 1)) - radius)));
        if (distance < radius){
            brush.mask.at(i) = 1;
        } else {
            brush.mask.at(i) = 0;
        }
    }
}

void Brush::create_linear_mask(int type_of_brush, int size_of_mask, int radius){
    for (int i = 0; i < size_of_mask; i ++){

        float distance;
        distance = sqrt(((round(i/(((2 * radius) + 1))) - radius) * (round(i/(((2 * radius) + 1))) - radius))
                        +  ((i%(((2 * radius) + 1)) - radius) * (i%(((2 * radius) + 1)) - radius)));
        float opacity;
        opacity = 1 - (distance / radius);
        if (opacity > 0){
            brush.mask.at(i) = opacity;
        } else{
            brush.mask.at(i) = 0;
        }
    }
}

void Brush::create_quadratic_mask(int type_of_brush, int size_of_mask, int radius){
    for (int i = 0; i < size_of_mask; i ++){

        float distance;
        distance = sqrt(((round(i/(((2 * radius) + 1))) - radius) * (round(i/(((2 * radius) + 1))) - radius))
                        +  ((i%(((2 * radius) + 1)) - radius) * (i%(((2 * radius) + 1)) - radius)));
        if (radius == 0){
            brush.mask.at(i) = 1;
        } else if (distance > radius){
            brush.mask.at(i) = 0;
        }
        else {
            float a;
            float b;
            b = (0.0 - 2.0) / (float)radius;
            a = 1.0 / ((float)radius * (float)radius);
            float opacity;
            opacity = (a * (distance * distance)) + (b * distance) + 1;
            brush.mask.at(i) = opacity;
        }
    }
}

// EXTRA CREDIT SPRAY BRUSH
//void Brush::create_spray_mask(int type_of_brush, int size_of_mask, int radius){
//       int dots_to_fill;
//       dots_to_fill = int((((2 * radius) - 1) * ((2 * radius) - 1)) * float(settings.brushDensity / 100.0));
//       int i;
//       while(dots_to_fill > 0){
//            i = arc4random_uniform(size_of_mask);
//            int distance;
//            distance = sqrt(((round(i/(((2 * radius) + 1))) - radius) * (round(i/(((2 * radius) + 1))) - radius))
//                            +  ((i%(((2 * radius) + 1)) - radius) * (i%(((2 * radius) + 1)) - radius)));
//            if (distance < radius){
//                brush.mask.at(i) = 1;
//                dots_to_fill = dots_to_fill - 1;
//            }
//       }
//}s
