#pragma once
#include "AMesh.h"
#include <map>
#include <string>
#include "AMeshLoader.h"

class ANVIL_API AResourceManager
{
  public:
    ~AResourceManager()
    {
        for (auto& p : m_meshes)
            delete p.second;
    }
    void RegisterMesh(const std::string& name, AMesh* mesh)
    {
        m_meshes[name] = mesh;
    }
    AMesh* GetMesh(const std::string& name)
    {
        return m_meshes[name];
    }

    AMesh* LoadMesh(const std::string& name, const std::string& path);

  private:
    std::map<std::string, AMesh*> m_meshes;
};