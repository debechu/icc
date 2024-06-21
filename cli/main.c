#include "args.h"
#include <icc/icc.h>
#include <sail/sail.h>
#include <sail-manip/sail-manip.h>
#include <stdlib.h>
#include <stdio.h>

int get_colors(const char *file, colors_t *out);

static const char help_string[] =
    "icc - image color clustering\n"
    "\n"
    "Usage:\n"
    "  icc <img-file>\n"
    "\n"
    "Options:\n"
    "  -h --help        Shows the help manual\n";

int main(int argc, const char **argv)
{
    args_t args = {0};
    if (parse_args(argc, argv, &args) != 0)
    {
        return 1;
    }

    if (argc == 1 || args.help)
    {
        fprintf(stderr, "%s", help_string);
        return 0;
    }

    if (!args.img_file)
    {
        fprintf(stderr, "Image file is not provided!\n");
        return 1;
    }

    colors_t colors = {0};
    if (get_colors(args.img_file, &colors) != 0)
    {
        fprintf(stderr, "Failed load image file: %s", args.img_file);
        return 1;
    }

    colors_t dominants = {0};
    if (get_dominant_colors(colors, &dominants) != 0)
    {
        fprintf(stderr, "Failed to get dominant colors!");
        return 1;
    }

    for (int i = 0; i < dominants.count; ++i)
    {
        color_t color = dominants.colors[i];
        fprintf(
            stderr,
            "\x1b[48;2;%d;%d;%dm  ",
            color.r, color.g, color.b
        );
    }
    fprintf(stderr, "\n");

    free(dominants.colors);
    free(colors.colors);

    return 0;
}

int get_colors(const char *file, colors_t *out)
{
    fprintf(stderr, "Loading image file: %s\n", file);

    struct sail_image *image = NULL;
    SAIL_TRY(sail_load_from_file(file, &image));
    {
        struct sail_image *image_rgb = NULL;
        SAIL_TRY(sail_convert_image(
            image,
            SAIL_PIXEL_FORMAT_BPP24_RGB,
            &image_rgb
        ));
        sail_destroy_image(image);
        image = image_rgb;
    }

    fprintf(stderr, "Image width: %d\n", image->width);
    fprintf(stderr, "Image height: %d\n", image->height);
    fprintf(stderr, "Image bytes per line: %d\n", image->bytes_per_line);
    fprintf(
        stderr,
        "Image pixel format: %s\n",
        sail_pixel_format_to_string(image->pixel_format)
    );

    size_t colors_count = image->width * image->height;
    color_t *colors = calloc(colors_count, sizeof(color_t));
    {
        uint8_t *data = image->pixels;
        for (int i = 0; i < colors_count; ++i)
        {
            size_t idx = i*3;
            colors[i].r = data[idx];
            colors[i].g = data[idx+1];
            colors[i].b = data[idx+2];
        }
    }

    sail_destroy_image(image);
    image = NULL;

    out->count = colors_count;
    out->colors = colors;

    return 0;
}
