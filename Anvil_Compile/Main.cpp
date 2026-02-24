#define STB_IMAGE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include "AnvilBSPFormat.h"
#include "AMeshLoader.h"
#include "AMesh.h"
#include "AMath.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <stb_image.h>

constexpr float SIZE = 0.1f;
namespace fs = std::filesystem;

// Some helpers
APlane PlaneFromPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    glm::vec3 v1 = p2 - p1, v2 = p3 - p1;
    glm::vec3 n = glm::normalize(glm::cross(v2, v1));
    return { n, glm::dot(n, p1) };
}

void ClipWinding(std::vector<glm::vec3>& pts, APlane& plane) {
    std::vector<glm::vec3> newPts;
    const float EPS = 0.01f;
    for (size_t i = 0; i < pts.size(); i++) {
        glm::vec3 p1 = pts[i], p2 = pts[(i + 1) % pts.size()];
        float d1 = glm::dot(p1, plane.normal) - plane.distance;
        float d2 = glm::dot(p2, plane.normal) - plane.distance;

        if (d1 <= EPS) newPts.push_back(p1);
        if ((d1 > EPS && d2 < -EPS) || (d1 < -EPS && d2 > EPS)) {
            float t = d1 / (d1 - d2);
            newPts.push_back(p1 + t * (p2 - p1));
        }
    }
    pts = newPts;
}
struct TempPlane {
    APlane plane;
    uint32_t texIdx;
};
struct EmbeddedTex {
    std::string name;
    uint32_t width;
    uint32_t height;
    uint32_t format; // 3 = RGB, 4 = RGBA
	std::vector<uint8_t> data;
};

// Big boy
// Add support for texturing
// Embedded textures would be nice
void CompileMap(const char* inputPath) {
    std::ifstream file(inputPath);
    if (!file) {
        std::cout << "[Anvil Compiler] Error: Map not found " << inputPath << std::endl;
        return;
    }

    std::string token, currentClassName = "";
    std::vector<AVertex> all_v;
    std::vector<AFace> all_f;
    std::vector<ABspEntity> entities;
    std::vector<APlane> all_planes;
    std::vector<ABSPBrush> all_brushes;
    std::vector<TempPlane> brushPlanes;
    std::vector<EmbeddedTex> textures;
    std::map<std::string, uint32_t> texNameToIndex;
    glm::vec3 entMin(1e9), entMax(-1e9);

    while (file >> token) {
        if (token == "\"classname\"") {
            file >> token;
            currentClassName = token.substr(1, token.length() - 2);
        }
        if (token == "(") {
            float x1, y1, z1, x2, y2, z2, x3, y3, z3;
            file >> x1 >> y1 >> z1; file >> token >> token;
            file >> x2 >> y2 >> z2; file >> token >> token;
            file >> x3 >> y3 >> z3; file >> token;

            std::string texName;
            file >> texName;

            if (texNameToIndex.find(texName) == texNameToIndex.end()) {
                uint32_t newIdx = (uint32_t)textures.size();
                texNameToIndex[texName] = newIdx;
                std::string imgPath = "textures/"+texName + ".png";
                int w, h, channels;
                uint8_t* pixels = stbi_load(imgPath.c_str(), &w, &h, &channels, 4);
                EmbeddedTex tex;
                tex.name = texName;
                if (pixels) {
                    tex.width = w;
                    tex.height = h;
                    tex.format = 4;
                    tex.data.assign(pixels, pixels + (w * h * 4));
					stbi_image_free(pixels);
                }
                else {
                    std::cout << "Warning: Could not load texture " << texName << std::endl;
                    tex.width = 2;
					tex.height = 2;
					tex.format = 4;
                    // don't mind reading this lmao, it's magenta black checkerboard
                    tex.data = { 255,0,255,255, 0,0,0,255, 0,0,0,255, 255,0,255,255 };
                }
                textures.push_back(tex);
            }
            uint32_t currentTexIdx = texNameToIndex[texName];
            // OpenGL and GLM only supports right-handed coordinate system
            // so, XYZ-> XZY
            glm::vec3 p1{ x1, z1, -y1 }, p2{ x2, z2, -y2 }, p3{ x3, z3, -y3 };
            brushPlanes.push_back({ PlaneFromPoints(p1, p2, p3) , currentTexIdx});

            entMin = glm::min(entMin, glm::min(p1, glm::min(p2, p3)));
            entMax = glm::max(entMax, glm::max(p1, glm::max(p2, p3)));

            std::string junk; std::getline(file, junk);
        }
        if (token == "}") {
            if (brushPlanes.size() >= 4) {
                ABSPBrush b;
                b.firstPlane = (uint32_t)all_planes.size();
                b.numPlanes = (uint32_t)brushPlanes.size();
                all_brushes.push_back(b);
                for (auto& p : brushPlanes) {
                    p.plane.distance *= SIZE;
					all_planes.push_back(p.plane);
                }

                for (size_t i = 0; i < brushPlanes.size(); i++) {
                    APlane& p = brushPlanes[i].plane;
                    uint32_t faceTexIdx = brushPlanes[i].texIdx;

                    glm::vec3 up = (std::abs(p.normal.y) > 0.99f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
                    glm::vec3 r = glm::normalize(glm::cross(p.normal, up));
                    up = glm::cross(r, p.normal);

                    std::vector<glm::vec3> w = {
                        (p.normal * p.distance) + (r * 1e4f) + (up * 1e4f),
                        (p.normal * p.distance) - (r * 1e4f) + (up * 1e4f),
                        (p.normal * p.distance) - (r * 1e4f) - (up * 1e4f),
                        (p.normal * p.distance) + (r * 1e4f) - (up * 1e4f)
                    };

                    for (size_t j = 0; j < brushPlanes.size(); j++) {
                        if (i != j) ClipWinding(w, brushPlanes[j].plane);
                    }

                    if (w.size() >= 3) {
                        all_f.push_back({ (uint32_t)all_v.size(), (uint32_t)w.size(), faceTexIdx });
                        for (auto& vPos : w) {
                            glm::vec2 uv = (std::abs(p.normal.y) > 0.5f) ? glm::vec2(vPos.x, vPos.z) : glm::vec2(vPos.x, vPos.y);                            // Applying 0.01f scale for engine units
                            all_v.push_back({ vPos * SIZE, uv * 0.01f, p.normal });
                        }
                    }
                }
            }
            if (currentClassName.find("trigger") != std::string::npos) {
                ABspEntity e;
                memset(e.name, 0, 64);
                strncpy(e.name, currentClassName.c_str(), 63);
                e.position = (entMin + entMax) * 0.5f * SIZE;
                e.size = (entMax - entMin) * SIZE;
                entities.push_back(e);
            }
            brushPlanes.clear();
            currentClassName = "";
            entMin = glm::vec3(1e9);
            entMax = glm::vec3(-1e9);
        }
    }
    std::ofstream out("world.absp", std::ios::binary);
    ABSPHeader h = { {'A','B','S','P'}, 2, (uint32_t)all_v.size(), (uint32_t)all_f.size(), (uint32_t)entities.size(), (uint32_t)all_planes.size(), (uint32_t)all_brushes.size() };
    out.write((char*)&h, sizeof(h));
    out.write((char*)all_v.data(), all_v.size() * sizeof(AVertex));
    out.write((char*)all_f.data(), all_f.size() * sizeof(AFace));
    out.write((char*)entities.data(), entities.size() * sizeof(ABspEntity));
    out.write((char*)all_planes.data(), all_planes.size() * sizeof(APlane));
    out.write((char*)all_brushes.data(), all_brushes.size() * sizeof(ABSPBrush));
    for (const auto& tex : textures) {
        ATextureEntry te;
        memset(te.name, 0, 64); // super safe
        strncpy(te.name, tex.name.c_str(), 63);
        te.width = tex.width;
        te.height = tex.height;
        te.format = tex.format;
        te.dataSize = (uint32_t)tex.data.size();
        out.write((char*)&te, sizeof(te));
    }
    for (const auto& tex : textures) {
        out.write((char*)tex.data.data(), tex.data.size());
    }
    std::cout << "[Anvil Compiler] Success: world.absp baked with " << all_brushes.size() << " brushes." << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: Anvil_Compile [map/mesh] [file]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string inputPath = argv[2];

    if (mode == "map") {
        CompileMap(inputPath.c_str());
    }
    else if (mode == "mesh") {
        fs::path p(inputPath);
        std::string outPath = p.replace_extension(".anvmesh").string();

        // Use the centralized loader class for the export!
        AMeshLoader::ExportToAnvMesh(inputPath, outPath);
    }

    return 0;
}