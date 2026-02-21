#pragma once
#include "ACore.h"
#include "AMesh.h"
#include "AShader.h"
#include <string>
#include <vector>

class ANVIL_API AModel
{
  public:
    AModel() = default;

    ~AModel()
    {
        for (auto* mesh : m_meshes)
        {
            delete mesh;
        }
    }

    void Draw()
    {
        for (auto* mesh : m_meshes)
        {
            mesh->Draw();
        }
    }

    int LoadFromFile(const std::string& path);
    void AddMesh(AMesh* mesh)
    {
        m_meshes.push_back(mesh);
    }

    const std::vector<AMesh*>& GetMeshes() const
    {
        return m_meshes;
    }
    AMesh* GetMesh(size_t index)
    {
        return m_meshes[index];
    }

  private:
    std::vector<AMesh*> m_meshes;
};