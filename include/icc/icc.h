#ifndef LIBICC_ICC_H
#define LIBICC_ICC_H

#include <stddef.h>
#include <stdint.h>

typedef struct color color_t;
struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef struct colors colors_t;
struct colors
{
    size_t count;
    color_t *colors;
};

int get_dominant_colors(colors_t colors, colors_t *out);

#endif
