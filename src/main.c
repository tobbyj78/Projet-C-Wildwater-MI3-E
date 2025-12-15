#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histo.h"
#include "leaks.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <mode> [plant_id]\n", argv[0]);
        fprintf(stderr, "Modes: max, src, real, leaks\n");
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *mode = argv[3];

    if (strcmp(mode, "max") == 0 || strcmp(mode, "src") == 0 || strcmp(mode, "real") == 0)
    {
        return histo_process(input_file, output_file, mode);
    }
    else if (strcmp(mode, "leaks") == 0)
    {
        if (argc < 5)
        {
            fprintf(stderr, "Error: leaks mode requires a plant identifier\n");
            return 1;
        }
        const char *plant_id = argv[4];
        return leaks_process(input_file, output_file, plant_id);
    }
    else
    {
        fprintf(stderr, "Error: Unknown mode '%s'\n", mode);
        return 1;
    }

    return 0;
}
