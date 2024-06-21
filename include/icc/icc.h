#ifndef LIBICC_ICC_H
#define LIBICC_ICC_H

#include <stddef.h>
#include <stdint.h>

typedef enum color_type color_type_t;
enum color_type
{
    ICC_COLOR_TYPE_RGB,
    ICC_COLOR_TYPE_HSV
};

typedef struct color_rgb color_rgb_t;
struct color_rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef struct color_hsv color_hsv_t;
struct color_hsv
{
    uint16_t h;
    uint8_t s;
    uint8_t v;
};

typedef union color color_t;
union color
{
    color_rgb_t rgb;
    color_hsv_t hsv;
};

typedef struct colors colors_t;
struct colors
{
    size_t count;
    color_t *colors;
    color_type_t type;
};

int
get_dominant_colors(
    colors_t *colors,
    color_type_t out_type, colors_t *out);

#endif
