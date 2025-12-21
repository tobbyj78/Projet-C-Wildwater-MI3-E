#include "leaks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cree un noeud d'arbre avec l'identifiant et le pourcentage de fuite
TreeNode *create_tree_node(char *id, float leak_pct)
{
    TreeNode *node = malloc(sizeof(TreeNode));
    if (node == NULL)
        return NULL;
    node->id = strdup(id);
    if (node->id == NULL)
    {
        free(node);
        return NULL;
    }
    node->leak_pct = leak_pct;
    node->child_count = 0;
    node->children = NULL;
    return node;
}

// Ajoute un fils à un noeud 
void add_child(TreeNode *parent, TreeNode *child)
{
    ChildNode *cn = malloc(sizeof(ChildNode));
    if (cn == NULL)
        return;
    cn->child = child;
    cn->next = parent->children;
    parent->children = cn;
    parent->child_count++;
}

// Libere un arbre et ses sous elements de maniere recursive
void free_tree(TreeNode *node)
{
    if (node == NULL)
        return;
    ChildNode *cn = node->children;
    while (cn)
    {
        ChildNode *next = cn->next;
        free_tree(cn->child);
        free(cn);
        cn = next;
    }
    free(node->id);
    free(node);
}

// Calcule les fuites pour un noeud avec le volume entrant de maniere recursive
float calculate_leaks_recursive(TreeNode *node, float incoming_volume)
{
    if (node == NULL || incoming_volume <= 0)
        return 0.0f;

    float leak = incoming_volume * (node->leak_pct / 100.0f);
    float remaining = incoming_volume - leak;

    if (node->child_count == 0)
    {
        return leak;
    }

    float volume_per_child = remaining / node->child_count;
    float total_leak = leak;

    ChildNode *cn = node->children;
    while (cn)
    {
        total_leak += calculate_leaks_recursive(cn->child, volume_per_child);
        cn = cn->next;
    }

    return total_leak;
}

// Traite le fichier d'entree pour calculer les fuites d'une usine et ecrit le résultat
int leaks_process(char *input_file, char *output_file, char *plant_id)
{
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        return 1;
    }

    AVLNode *node_index = NULL;
    TreeNode *plant_root = NULL;
    float plant_max_volume = 0.0f;
    int plant_found = 0;

    char line[1024];
    char **lines = NULL;
    int line_count = 0;
    int line_capacity = 10000;

    lines = malloc(sizeof(char *) * line_capacity);
    if (lines == NULL)
    {
        fclose(fp);
        return 1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (line_count >= line_capacity)
        {
            line_capacity *= 2;
            char **new_lines = realloc(lines, sizeof(char *) * line_capacity);
            if (new_lines == NULL)
            {
                for (int i = 0; i < line_count; i++)
                    free(lines[i]);
                free(lines);
                fclose(fp);
                return 1;
            }
            lines = new_lines;
        }
        lines[line_count] = strdup(line);
        line_count++;
    }
    fclose(fp);

    for (int i = 0; i < line_count; i++)
    {
        char *l = lines[i];
        l[strcspn(l, "\n\r")] = 0;

        char col1[256] = "-", col2[256] = "-", col3[256] = "-", col4[256] = "-", col5[256] = "-";
        char *lcopy = strdup(l);

        char *token = strtok(lcopy, ";");
        if (token)
            strncpy(col1, token, sizeof(col1) - 1);
        token = strtok(NULL, ";");
        if (token)
            strncpy(col2, token, sizeof(col2) - 1);
        token = strtok(NULL, ";");
        if (token)
            strncpy(col3, token, sizeof(col3) - 1);
        token = strtok(NULL, ";");
        if (token)
            strncpy(col4, token, sizeof(col4) - 1);
        token = strtok(NULL, ";");
        if (token)
            strncpy(col5, token, sizeof(col5) - 1);

        free(lcopy);

        if (strcmp(col1, "-") == 0 && strcmp(col2, plant_id) == 0 && strcmp(col3, "-") == 0 && strcmp(col4, "-") != 0)
        {
            plant_max_volume = atof(col4);
            plant_found = 1;
        }
    }

    if (!plant_found)
    {
        for (int i = 0; i < line_count; i++)
            free(lines[i]);
        free(lines);

        FILE *out = fopen(output_file, "a");
        if (out == NULL)
        {
            fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
            return 1;
        }

        fseek(out, 0, SEEK_END);
        long pos = ftell(out);
        if (pos == 0)
        {
            fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");
        }
        fprintf(out, "%s;-1\n", plant_id);
        fclose(out);
        return 0;
    }

    plant_root = create_tree_node(plant_id, 0.0f);
    if (plant_root == NULL)
    {
        for (int i = 0; i < line_count; i++)
            free(lines[i]);
        free(lines);
        return 1;
    }

    AVLNode *found = NULL;
    node_index = avl_insert(node_index, plant_id, plant_root, &found);

    int changes = 1;
    while (changes)
    {
        changes = 0;
        for (int i = 0; i < line_count; i++)
        {
            char *l = lines[i];
            char col1[256] = "-", col2[256] = "-", col3[256] = "-", col4[256] = "-", col5[256] = "-";
            char *lcopy = strdup(l);

            char *token = strtok(lcopy, ";");
            if (token)
                strncpy(col1, token, sizeof(col1) - 1);
            token = strtok(NULL, ";");
            if (token)
                strncpy(col2, token, sizeof(col2) - 1);
            token = strtok(NULL, ";");
            if (token)
                strncpy(col3, token, sizeof(col3) - 1);
            token = strtok(NULL, ";");
            if (token)
                strncpy(col4, token, sizeof(col4) - 1);
            token = strtok(NULL, ";");
            if (token)
                strncpy(col5, token, sizeof(col5) - 1);

            free(lcopy);

            if (strcmp(col3, "-") == 0)
                continue;

            int is_plant_segment = (strcmp(col1, plant_id) == 0);
            int is_plant_to_storage = (strcmp(col1, "-") == 0 && strcmp(col2, plant_id) == 0);

            if (!is_plant_segment && !is_plant_to_storage)
                continue;

            AVLNode *parent_node = avl_search(node_index, col2);
            if (parent_node == NULL)
                continue;

            AVLNode *child_node = avl_search(node_index, col3);
            if (child_node != NULL)
                continue;

            float leak_pct = (strcmp(col5, "-") != 0) ? atof(col5) : 0.0f;
            TreeNode *child = create_tree_node(col3, leak_pct);
            if (child == NULL)
                continue;

            TreeNode *parent = (TreeNode *)parent_node->data;
            add_child(parent, child);

            node_index = avl_insert(node_index, col3, child, NULL);
            changes = 1;
        }
    }

    float total_leaks = 0.0f;

    if (plant_root->child_count > 0)
    {
        float volume_per_child = plant_max_volume / plant_root->child_count;
        ChildNode *cn = plant_root->children;
        while (cn)
        {
            total_leaks += calculate_leaks_recursive(cn->child, volume_per_child);
            cn = cn->next;
        }
    }

    float total_leaks_mm3 = total_leaks / 1000000.0f;

    FILE *out = fopen(output_file, "a");
    if (out == NULL)
    {
        fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
        free_tree(plant_root);
        avl_free(node_index, NULL);
        for (int i = 0; i < line_count; i++)
            free(lines[i]);
        free(lines);
        return 1;
    }

    fseek(out, 0, SEEK_END);
    long pos = ftell(out);
    if (pos == 0)
    {
        fprintf(out, "identifier;Leak volume (M.m3.year-1)\n");
    }
    fprintf(out, "%s;%.6f\n", plant_id, total_leaks_mm3);
    fclose(out);

    free_tree(plant_root);
    avl_free(node_index, NULL);
    for (int i = 0; i < line_count; i++)
        free(lines[i]);
    free(lines);

    return 0;
}
