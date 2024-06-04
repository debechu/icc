#ifndef ICC_CLI_ARGS_H
#define ICC_CLI_ARGS_H

#include <stdbool.h>

typedef struct args args_t;
struct args
{
    const char *img_file;
    bool help;
};

int parse_args(int argc, const char **argv, args_t *out);

#endif
