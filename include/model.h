#pragma once
#include "int.h"
#include "vertex.h"

typedef struct Model{
    u32 verticesCount;
    Vertex *vertices;
    u32 indicesCount;
    u16 *indices;
} Model;

Model loadModel(const char *filePath);
void freeModel(Model *model);