#include "histo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void free_plant_data(void *data)
{
    if (data)
    {
        PlantData *pd = (PlantData *)data;
        free(pd->id);
        free(pd);
    }
}

static int is_source(const char *id)
{
    return (strstr(id, "Source") != NULL ||
            strstr(id, "Well") != NULL ||
            strstr(id, "Spring") != NULL ||
            strstr(id, "Fountain") != NULL ||
            strstr(id, "Resurgence") != NULL ||
            strstr(id, "field") != NULL);
}

static int is_plant(const char *id)
{
    return (strstr(id, "Plant") != NULL ||
            strstr(id, "Unit") != NULL ||
            strstr(id, "Module") != NULL ||
            strstr(id, "Facility") != NULL);
}

typedef struct
{
    FILE *fp;
    const char *mode;
} WriteContext;

static void write_plant(AVLNode *node, void *arg)
{
    WriteContext *ctx = (WriteContext *)arg;
    PlantData *pd = (PlantData *)node->data;
    double value = 0.0;

    if (strcmp(ctx->mode, "max") == 0)
    {
        value = pd->max_volume / 1000.0;
    }
    else if (strcmp(ctx->mode, "src") == 0)
    {
        value = pd->source_volume / 1000.0;
    }
    else if (strcmp(ctx->mode, "real") == 0)
    {
        value = pd->real_volume / 1000.0;
    }

    fprintf(ctx->fp, "%s;%.6f\n", pd->id, value);
}

int histo_process(const char *input_file, const char *output_file, const char *mode)
{
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        return 1;
    }

    AVLNode *plants = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n\r")] = 0;

        char col1[256] = "-", col2[256] = "-", col3[256] = "-", col4[256] = "-", col5[256] = "-";

        char *token = strtok(line, ";");
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

        if (strcmp(col1, "-") == 0 && is_plant(col2) && strcmp(col3, "-") == 0 && strcmp(col4, "-") != 0)
        {
            AVLNode *found = NULL;
            plants = avl_insert(plants, col2, NULL, &found);
            if (found && found->data == NULL)
            {
                PlantData *pd = malloc(sizeof(PlantData));
                if (pd == NULL)
                {
                    fclose(fp);
                    avl_free(plants, free_plant_data);
                    return 1;
                }
                pd->id = strdup(col2);
                pd->max_volume = atof(col4);
                pd->source_volume = 0.0;
                pd->real_volume = 0.0;
                found->data = pd;
            }
            else if (found && found->data)
            {
                PlantData *pd = (PlantData *)found->data;
                pd->max_volume = atof(col4);
            }
        }

        if (strcmp(col1, "-") == 0 && is_source(col2) && is_plant(col3) && strcmp(col4, "-") != 0)
        {
            double volume = atof(col4);
            double leak_pct = (strcmp(col5, "-") != 0) ? atof(col5) : 0.0;
            double real = volume * (1.0 - leak_pct / 100.0);

            AVLNode *found = NULL;
            plants = avl_insert(plants, col3, NULL, &found);
            if (found)
            {
                if (found->data == NULL)
                {
                    PlantData *pd = malloc(sizeof(PlantData));
                    if (pd == NULL)
                    {
                        fclose(fp);
                        avl_free(plants, free_plant_data);
                        return 1;
                    }
                    pd->id = strdup(col3);
                    pd->max_volume = 0.0;
                    pd->source_volume = volume;
                    pd->real_volume = real;
                    found->data = pd;
                }
                else
                {
                    PlantData *pd = (PlantData *)found->data;
                    pd->source_volume += volume;
                    pd->real_volume += real;
                }
            }
        }
    }
    fclose(fp);

    FILE *out = fopen(output_file, "w");
    if (out == NULL)
    {
        fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
        avl_free(plants, free_plant_data);
        return 1;
    }

    if (strcmp(mode, "max") == 0)
    {
        fprintf(out, "identifier;max volume (k.m3.year-1)\n");
    }
    else if (strcmp(mode, "src") == 0)
    {
        fprintf(out, "identifier;source volume (k.m3.year-1)\n");
    }
    else if (strcmp(mode, "real") == 0)
    {
        fprintf(out, "identifier;real volume (k.m3.year-1)\n");
    }

    WriteContext ctx = {out, mode};
    avl_traverse_reverse(plants, write_plant, &ctx);

    fclose(out);
    avl_free(plants, free_plant_data);
    return 0;
}
