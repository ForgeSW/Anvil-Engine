#include "AResourceManager.h"

AMesh* AResourceManager::LoadMesh(const std::string& name, const std::string& path)
{
    if (m_meshes.count(name))
        return m_meshes[name];

    AMesh* mesh = AMeshLoader::LoadAnvMesh(path);
    if (mesh)
        m_meshes[name] = mesh;
    return mesh;
}