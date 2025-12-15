#ifndef HISTO_H
#define HISTO_H

#include "avl.h"

typedef struct
{
    char *id;
    double max_volume;
    double source_volume;
    double real_volume;
} PlantData;

int histo_process(const char *input_file, const char *output_file, const char *mode);

#endif
