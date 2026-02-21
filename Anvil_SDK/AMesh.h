#pragma once
#include "ACore.h"
#include "AMath.h"
#include <vector>
#include <cstdint>

struct MVertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
};

/**
 * @class AMesh
 * @brief Represents a mesh object in the ANVIL engine with vertices, indices, and texture support.
 */
class ANVIL_API AMesh
{
  public:
    AMesh(std::vector<MVertex> verts, std::vector<uint32_t> indices, uint32_t texID = 0);
    ~AMesh();
    void Draw();

  private:
    uint32_t VAO, VBO, EBO;
    uint32_t indexCount;
    uint32_t m_textureID;
};