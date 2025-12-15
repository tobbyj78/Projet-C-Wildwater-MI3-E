#ifndef AVL_H
#define AVL_H

#include <stdlib.h>

typedef struct AVLNode
{
    char *key;
    void *data;
    int equilibre;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

AVLNode *avl_create_node(const char *key, void *data);
AVLNode *avl_insert(AVLNode *root, const char *key, void *data, AVLNode **found);
AVLNode *avl_search(AVLNode *root, const char *key);
void avl_traverse_reverse(AVLNode *root, void (*callback)(AVLNode *, void *), void *arg);
void avl_free(AVLNode *root, void (*free_data)(void *));
int avl_count(AVLNode *root);
AVLNode *rotate_left(AVLNode *a);
AVLNode *rotate_right(AVLNode *a);
AVLNode *double_rotate_left(AVLNode *a);
AVLNode *double_rotate_right(AVLNode *a);
AVLNode *balance_avl(AVLNode *a);

#endif
