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
    double leak_pct;
    int child_count;
    ChildNode *children;
} TreeNode;

int leaks_process(const char *input_file, const char *output_file, const char *plant_id);

#endif
