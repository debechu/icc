#include "args.h"

#include <stdio.h>

static const char help_string[] =
    "icc - image color clustering\n"
    "\n"
    "Usage:\n"
    "  icc <img-file>]\n"
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

    fprintf(stderr, "Image file: %s\n", args.img_file);

    return 0;
}
