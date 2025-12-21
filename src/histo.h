#ifndef HISTO_H
#define HISTO_H

#include "avl.h"

typedef struct
{
    char *id;
    float max_volume;
    float source_volume;
    float real_volume;
} PlantData;

int histo_process(char *input_file, char *output_file, char *mode);

#endif
