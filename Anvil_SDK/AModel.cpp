#include "AModel.h"
#include "AMeshLoader.h"
#include <iostream>

/**
 * Load a 3D model from a file and add it to the model's mesh collection
 * @param path The file path of the model to be loaded
 * @return 1 if loading was successful, 0 if loading failed
 */
int AModel::LoadFromFile(const std::string& path)
{
    // Attempt to load a mesh from the specified file path
    AMesh* mesh = AMeshLoader::LoadAnvMesh(path);
    if (mesh)
    {
        // If mesh loading was successful, add it to the meshes vector
        m_meshes.push_back(mesh);
        return 1;
    }
    std::cout << " Model Error: Failed To Load " << path << "\n";
    return 0;
}
