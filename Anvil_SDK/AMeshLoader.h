#pragma once
#include "AMesh.h"
#include <string>
#include <vector>

// mesh.anvmesh
struct AMeshHeader
{
    uint32_t numVertices = 0;
    uint32_t numIndices = 0;
    uint32_t pathLength = 0;
};

class ANVIL_API AMeshLoader
{
  public:
    static void ExportToAnvMesh(const std::string& inputPath, const std::string& outputPath);

    static AMesh* LoadAnvMesh(const std::string& path);
};