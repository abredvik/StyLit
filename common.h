#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <cstdint>

//Canvas2D c;
class Common
{
public:

    int m_width;

    int m_height;

    void set(int w, int h);

    int pos_to_index(int x, int y, int width);

    float int_to_float(std::uint8_t i);

    std::uint8_t float_to_int(float f);

    bool is_in_range(int x, int y);
};

extern Common common;
#endif // COMMON_H
