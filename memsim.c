#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memsim.h"

int main(int argc, char *argv[])
{
    if (argc < 5 || argc > 6)
    {
        printf("Invalid number of arguments!\n");
        printf("Usage: memsim <tracefile> <nframes> <lru|fifo|vms> [<p>] <debug|quiet>\n");
        return 1;
    }

    char *tracefile_name = argv[1];
    int nframes = atoi(argv[2]);
    char *algorithm = argv[3];
    int p = 0;
    int debug = 0;

    if (argc == 6)
    {
        p = atoi(argv[4]);
        debug = strcmp(argv[5], "debug") == 0;
    }
    else
    {
        debug = strcmp(argv[4], "debug") == 0;
    }

    FILE *tracefile = fopen(tracefile_name, "r");
    if (!tracefile)
    {
        printf("Failed to open tracefile: %s\n", tracefile_name);
        return 1;
    }

    if (strcmp(algorithm, "fifo") == 0)
    {
        fifo(tracefile, nframes, debug);
    }
    else if (strcmp(algorithm, "lru") == 0)
    {
        lru(tracefile, nframes, debug);
    }
    else if (strcmp(algorithm, "vms") == 0)
    {
        if (p < 0 || p > 100)
        {
            printf("Invalid value for 'p' in VMS algorithm. Must be between 0 and 100.\n");
            return 1;
        }
        segmented_fifo(tracefile, nframes, p, debug);
    }
    else
    {
        printf("Invalid page replacement algorithm: %s\n", algorithm);
        return 1;
    }

    fclose(tracefile);
    return 0;
}
