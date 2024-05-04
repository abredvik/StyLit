#include "common.h"

/**
 * @brief Canvas2D::pos_to_index returns an index based on the given x and y coordinates
 * @param x
 * @param y
 * @param width
 */
int Common::pos_to_index(int x, int y, int width) {
    int index = (y * width) + x;
    return index;
}

/**
 * @brief resets the values stored withing Common that represent out canvas's width and height
 * @param w represents width
 * @param h represents height
 */
void Common::set(int w, int h){
    m_width = w;
    m_height = h;
}

/**
 * @brief Canvas2D::int_to_float returns a float corresponding to the given uint8_t
 * @param interger
 */
float Common::int_to_float(std::uint8_t integer) {
    return integer / float(255);
}

/**
 * @brief Canvas2D::float_to_int returns a integer corresponding to the given float
 * @param f
 */
std::uint8_t Common::float_to_int(float f) {
    return round(255 * f);
}

/**
 * @brief Canvas2D::is_in_range returns true if the entire mask of the brush is within the canvas's bounds
 * @param x
 * @param y
 */
bool Common::is_in_range(int x, int y){
    return ((x <= m_width - 1) && (x >= 0 )
            && (y <= m_height - 1) && (y >= 0));
}
