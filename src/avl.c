#include "avl.h"
#include <string.h>
#include <stdio.h>

AVLNode *avl_create_node(const char *key, void *data)
{
    AVLNode *node = malloc(sizeof(AVLNode));
    if (node == NULL)
        return NULL;
    node->key = strdup(key);
    if (node->key == NULL)
    {
        free(node);
        return NULL;
    }
    node->data = data;
    node->equilibre = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

AVLNode *rotate_left(AVLNode *a)
{
    AVLNode *b = a->right;

    a->right = b->left;
    b->left = a;

    if (b->equilibre >= 0)
    {
        a->equilibre = a->equilibre - 1 - b->equilibre;
        b->equilibre = b->equilibre - 1;
    }
    else
    {
        a->equilibre = a->equilibre - 1;
        b->equilibre = b->equilibre - 1 + a->equilibre;
    }

    return b;
}

AVLNode *rotate_right(AVLNode *a)
{
    AVLNode *b = a->left;

    a->left = b->right;
    b->right = a;

    if (b->equilibre <= 0)
    {
        a->equilibre = a->equilibre + 1 - b->equilibre;
        b->equilibre = b->equilibre + 1;
    }
    else
    {
        a->equilibre = a->equilibre + 1;
        b->equilibre = b->equilibre + 1 + a->equilibre;
    }

    return b;
}

AVLNode *double_rotate_left(AVLNode *a)
{
    a->right = rotate_right(a->right);
    return rotate_left(a);
}

AVLNode *double_rotate_right(AVLNode *a)
{
    a->left = rotate_left(a->left);
    return rotate_right(a);
}

AVLNode *balance_avl(AVLNode *a)
{
    if (a->equilibre == 2)
    {
        if (a->right->equilibre >= 0)
            return rotate_left(a);
        else
            return double_rotate_left(a);
    }

    if (a->equilibre == -2)
    {
        if (a->left->equilibre <= 0)
            return rotate_right(a);
        else
            return double_rotate_right(a);
    }

    return a;
}

AVLNode *avl_insert_recursive(AVLNode *a, const char *key, void *data, int *h, AVLNode **found)
{
    if (a == NULL)
    {
        *h = 1;
        AVLNode *node = avl_create_node(key, data);
        if (found)
            *found = node;
        return node;
    }

    int cmp = strcmp(key, a->key);
    if (cmp < 0)
    {
        a->left = avl_insert_recursive(a->left, key, data, h, found);
        *h = -*h;
    }
    else if (cmp > 0)
    {
        a->right = avl_insert_recursive(a->right, key, data, h, found);
    }
    else
    {
        *h = 0;
        if (found)
            *found = a;
        return a;
    }

    if (*h != 0)
    {
        a->equilibre += *h;

        if (a->equilibre == 0)
            *h = 0;
        else if (a->equilibre == 2 || a->equilibre == -2)
        {
            a = balance_avl(a);
            *h = 0;
        }
        else
        {
            *h = 1;
        }
    }

    return a;
}

AVLNode *avl_insert(AVLNode *root, const char *key, void *data, AVLNode **found)
{
    int h = 0;
    return avl_insert_recursive(root, key, data, &h, found);
}

AVLNode *avl_search(AVLNode *root, const char *key)
{
    if (root == NULL)
        return NULL;
    int cmp = strcmp(key, root->key);
    if (cmp < 0)
        return avl_search(root->left, key);
    if (cmp > 0)
        return avl_search(root->right, key);
    return root;
}

void avl_traverse_reverse(AVLNode *root, void (*callback)(AVLNode *, void *), void *arg)
{
    if (root == NULL)
        return;
    avl_traverse_reverse(root->right, callback, arg);
    callback(root, arg);
    avl_traverse_reverse(root->left, callback, arg);
}

void avl_free(AVLNode *root, void (*free_data)(void *))
{
    if (root == NULL)
        return;
    avl_free(root->left, free_data);
    avl_free(root->right, free_data);
    free(root->key);
    if (free_data && root->data)
        free_data(root->data);
    free(root);
}

int avl_count(AVLNode *root)
{
    if (root == NULL)
        return 0;
    return 1 + avl_count(root->left) + avl_count(root->right);
}
