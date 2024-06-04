#include "args.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// TODO(debe): Properly parse the command line arguments.
int
parse_args(int argc, const char **argv, args_t *out)
{
    assert(argv != NULL);
    assert(out != NULL);

    // Ignore the command
    --argc;
    ++argv;

    args_t args = {0};
    while (argc > 0)
    {
        const char *arg = *argv;
        switch (arg[0])
        {
            case '-':
                if (arg[1] == '\0')
                {
                    fprintf(
                        stderr,
                        "Invalid empty option '-'\n"
                    );
                    return 1;
                }

                if (arg[1] == '-')
                {
                    if (strncmp(arg+2, "help", 4) == 0)
                    {
                        args.help = true;
                        goto loop_end;
                    }

                    fprintf(
                        stderr,
                        "Unknown option '%s'\n",
                        arg
                    );
                    return 1;
                }

                for (int i = 1; arg[i] != '\0'; ++i)
                {
                    switch (arg[i])
                    {
                        case 'h':
                            args.help = true;
                            break;
                        default:
                            fprintf(
                                stderr,
                                "Unknown option '-%c'\n",
                                arg[i]
                            );
                            return 1;
                    }
                }
                break;

            default:
                if (args.img_file)
                {
                    fprintf(
                        stderr,
                        "Invalid extra argument, <img-file> is already given: '%s'\n",
                        args.img_file
                    );
                    return 1;
                }
                args.img_file = arg;
                break;
        }

    loop_end:
        --argc;
        ++argv;
    }

    *out = args;

    return 0;
}
