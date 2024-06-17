#include "args.h"
#include "sail-common/common_serialize.h"

#include <sail/sail.h>
#include <stdio.h>

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

    fprintf(stderr, "Loading image file: %s\n", args.img_file);

    struct sail_image *image = NULL;
    SAIL_TRY(sail_load_from_file(args.img_file, &image));

    fprintf(stderr, "Image width: %d\n", image->width);
    fprintf(stderr, "Image height: %d\n", image->height);
    fprintf(stderr, "Image bytes per line: %d\n", image->bytes_per_line);
    fprintf(
        stderr,
        "Image pixel format: %s\n",
        sail_pixel_format_to_string(image->pixel_format)
    );

    sail_destroy_image(image);
    image = NULL;

    return 0;
}
