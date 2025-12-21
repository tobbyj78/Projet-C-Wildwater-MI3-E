#ifndef LEAKS_H
#define LEAKS_H

#include "avl.h"

typedef struct ChildNode
{
    struct TreeNode *child;
    struct ChildNode *next;
} ChildNode;

typedef struct TreeNode
{
    char *id;
    float leak_pct;
    int child_count;
    ChildNode *children;
} TreeNode;

int leaks_process(char *input_file, char *output_file, char *plant_id);

#endif
