#define STB_IMAGE_IMPLEMENTATION
#include "AMeshLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>
#include <string>
#include <vector>
#include <filesystem>

/**
 * @brief Loads a texture from file and creates an OpenGL texture object
 * @param texturePath Path to the texture file
 * @return OpenGL texture ID (0 if loading failed)
 */
uint32_t LoadTextureAuto(const std::string& texturePath)
{
    int            w, h, channels;  // Width, height, and number of channels for the image
    unsigned char* pixels = stbi_load(texturePath.c_str(), &w, &h, &channels, 4); // Load image with 4 channels (RGBA)

    if (pixels)  // Check if image loading was successful
    {
        unsigned int texID;  // Variable to store the texture ID
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);  // Bind the texture

        // Create texture and generate mipmaps
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(pixels);
        std::cout << "[Anvil Engine] Success: Loaded texture " << texturePath << std::endl;
        return texID;
    }

    std::cout << "[Anvil Engine] Warning: No texture found for " << texturePath
              << ". Using default white." << std::endl;
    return 0;
}

/**
 * Loads a mesh from a file in the Anvil Engine mesh format.
 * @param path The path to the mesh file.
 * @return A pointer to the loaded AMesh object, or nullptr if loading failed.
 */
AMesh* AMeshLoader::LoadAnvMesh(const std::string& path)
{
    // Open the file in binary mode
    std::ifstream is(path, std::ios::binary);
    if (!is)
    {
        // Print error message if file couldn't be opened
        std::cout << "[Anvil Engine] Error: Could not open " << path << std::endl;
        return nullptr;
    }

    // Read mesh header from the file
    AMeshHeader head;
    is.read((char*) &head, sizeof(AMeshHeader));

    // Read vertex data from the file
    std::vector<MVertex> vertices(head.numVertices);
    is.read((char*) vertices.data(), head.numVertices * sizeof(MVertex));

    // Read index data from the file
    std::vector<uint32_t> indices(head.numIndices);
    is.read((char*) indices.data(), head.numIndices * sizeof(uint32_t));

    // Read texture path if present
    std::string texName = "";
    if (head.pathLength > 0)
    {
        std::vector<char> pathBuf(head.pathLength);
        is.read(pathBuf.data(), head.pathLength);
        texName = std::string(pathBuf.data(), head.pathLength);
    }
    // Close the file
    is.close();

    // Load texture if a texture path was provided
    uint32_t texID = 0;
    if (!texName.empty())
    {
        // Get the directory of the model file
        std::filesystem::path modelPath(path);
        // Construct the full path to the texture file
        std::filesystem::path textureFullPath = modelPath.parent_path() / texName;

        // Load the texture and get its ID
        texID = LoadTextureAuto(textureFullPath.string());
    }

    // Create and return a new AMesh object with the loaded data
    return new AMesh(vertices, indices, texID);
}
// Anvil_Compile loves this
/**
 * @brief Exports a 3D mesh to AnvMesh format
 * @param input Path to the input mesh file
 * @param output Path to the output AnvMesh file
 */
void AMeshLoader::ExportToAnvMesh(const std::string& input, const std::string& output)
{
    Assimp::Importer importer;
    // Process everything: Triangulate, Flip UVs for OpenGL, and Join identical vertices to save space
    // space
    const aiScene* scene =
        importer.ReadFile(input, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                     aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode)
    {
        std::cout << "[Anvil Compiler] Assimp Error: " << importer.GetErrorString() << std::endl;
        return;
    }

    std::vector<MVertex>  allVertices;
    std::vector<uint32_t> allIndices;
    std::string           textureFileName = "";

    // 1. EXTRACT TEXTURE (Embedded or External)
    if (scene->HasMaterials())
    {
        aiMaterial* mat = scene->mMaterials[0];
        aiString    str;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS)
        {
            const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str());

            if (embeddedTex)
            {
                std::filesystem::path outPath(output);
                textureFileName =
                    outPath.stem().string() + "_embedded." +
                    (embeddedTex->achFormatHint[0] ? embeddedTex->achFormatHint : "png");
                std::filesystem::path finalTexPath = outPath.parent_path() / textureFileName;

                std::ofstream texOut(finalTexPath, std::ios::binary);
                if (embeddedTex->mHeight == 0)
                {
                    texOut.write((char*) embeddedTex->pcData, embeddedTex->mWidth);
                }
                else
                {
                    std::cout << "[Anvil Compiler] Warning: Raw ARGB texture not extracted. Use "
                                 "PNG/JPG embedded."
                              << std::endl;
                }
                texOut.close();
            }
            else
            {
                std::filesystem::path p(str.C_Str());
                textureFileName = p.filename().string();
            }
        }
    }

    // 2. PROCESS ALL MESHES (Fixes the "4 verts" crime)
    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh*  mesh       = scene->mMeshes[m];
        uint32_t vertOffset = (uint32_t) allVertices.size();

        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MVertex v;
            v.pos    = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            v.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

            if (mesh->mTextureCoords[0])
                v.uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            else
                v.uv = {0.0f, 0.0f};

            allVertices.push_back(v);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                allIndices.push_back(face.mIndices[j] + vertOffset);
        }
    }

    // 3. WRITE TO ANVMESH
    std::ofstream os(output, std::ios::binary);
    AMeshHeader   header;
    header.numVertices = (uint32_t) allVertices.size();
    header.numIndices  = (uint32_t) allIndices.size();
    header.pathLength  = (uint32_t) textureFileName.length();

    os.write((char*) &header, sizeof(AMeshHeader));
    os.write((char*) allVertices.data(), allVertices.size() * sizeof(MVertex));
    os.write((char*) allIndices.data(), allIndices.size() * sizeof(uint32_t));
    if (header.pathLength > 0)
        os.write(textureFileName.c_str(), header.pathLength);

    os.close();

    std::cout << "[Anvil Compiler] SUCCESS: " << output << std::endl;
    std::cout << "  - Vertices: " << allVertices.size() << std::endl;
    std::cout << "  - Texture: " << (textureFileName.empty() ? "NONE" : textureFileName)
              << std::endl;
}