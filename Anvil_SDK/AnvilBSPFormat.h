#pragma once
#include "AMath.h"
#include <vector>
#include <cstdint>

struct ABSPBrush
{
    uint32_t firstPlane;
    uint32_t numPlanes;
};

struct ABspEntity
{
    char name[64];
    glm::vec3 position;
    glm::vec3 size;

};
struct ABSPHeader
{
    char     magic[4]; // "ABSP"
    uint32_t version;  // 2
    uint32_t numVertices;
    uint32_t numFaces;
    uint32_t numEntities;
    uint32_t numPlanes;
    uint32_t numBrushes;
    uint32_t numTextures;
};

struct AVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct AFace
{
    uint32_t firstVertex;
    uint32_t numVertices;
    uint32_t textureID;
};
struct APlane
{
    glm::vec3 normal;
    float     distance;
};

struct ATextureEntry {
    char name[64]; // texture name from the .map file , like "float/wood1" or sum like that
	uint32_t width;
	uint32_t height;
	uint32_t format; // GL_RGB or GL_RGBA
    uint32_t dataSize;
};