#include "AShader.h"
#include "resource.h"
#include <Windows.h>
#include <glad/glad.h>
#include <iostream>

/**
 * Loads shader code from a resource in the Anvil_SDK.dll module
 * @param resID The resource ID of the shader to load
 * @return A string containing the shader code, or empty string if resource not found
 */
std::string AShader::LoadFromResource(int resID)
{
    // Get handle to the Anvil_SDK.dll module
    HMODULE hMod = GetModuleHandleA("Anvil_SDK.dll");
    // Find the shader resource by ID and type
    HRSRC   hRes = FindResourceA(hMod, (LPCSTR) MAKEINTRESOURCE(resID), "SHADER");
    // Return empty string if resource not found
    if (!hRes)
        return "";
    // Load the resource data into memory
    HGLOBAL hData = LoadResource(hMod, hRes);
    // Get the size of the resource data
    DWORD   size  = SizeofResource(hMod, hRes);
    // Create and return a string from the resource data
    return std::string((const char*) LockResource(hData), size);
}

/**
 * Constructor for AShader class that initializes a shader program by loading vertex and fragment shaders from resources.
 * @param vertResID Resource ID for the vertex shader
 * @param fragResID Resource ID for the fragment shader
 */
AShader::AShader(int vertResID, int fragResID)
{
    // Load vertex and fragment shader source code from resources
    std::string v = LoadFromResource(vertResID);
    std::string f = LoadFromResource(fragResID);

    // Check if either shader source code is empty
    if (v.empty() || f.empty())
    {
        // Output error message if resources couldn't be found
        std::cout << "[Anvil Shader] Critical: Could not find resource " << vertResID << " or "
                  << fragResID << std::endl;
        return;
    }

    // Compile the shader program with the loaded vertex and fragment shader sources
    Compile(v.c_str(), f.c_str());
}

/**
 * Compiles and links vertex and fragment shaders into a shader program
 * @param vCode C-string containing the source code of the vertex shader
 * @param fCode C-string containing the source code of the fragment shader
 */
void AShader::Compile(const char* vCode, const char* fCode)
{
    uint32_t v, f; // Handles for vertex and fragment shaders

    // Vertex Shader setup and compilation
    v = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader object
    glShaderSource(v, 1, &vCode, NULL); // Attach vertex shader source code
    glCompileShader(v); // Compile the vertex shader
    CheckCompileErrors(v, "VERTEX"); // Check for compilation errors

    // Fragment Shader setup and compilation
    f = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader object
    glShaderSource(f, 1, &fCode, NULL); // Attach fragment shader source code
    glCompileShader(f); // Compile the fragment shader
    CheckCompileErrors(f, "FRAGMENT"); // Check for compilation errors

    // Shader Program creation and linking
    m_ID = glCreateProgram(); // Create shader program
    glAttachShader(m_ID, v); // Attach vertex shader to program
    glAttachShader(m_ID, f); // Attach fragment shader to program
    glLinkProgram(m_ID);
    CheckCompileErrors(m_ID, "PROGRAM"); // Check for linking errors

    // Clean up - delete the shader objects as they're linked into the program
    glDeleteShader(v);
    glDeleteShader(f);
}

void AShader::CheckCompileErrors(uint32_t shader, std::string type)
{
    int  success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "| ERROR: " << type << " COMPILATION FAILED\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "| ERROR: PROGRAM LINKING FAILED\n" << infoLog << std::endl;
        }
    }
}

/**
 * Activates the shader program
 * This function makes the shader program the active one for rendering operations
 */
void AShader::Use()
{
    glUseProgram(m_ID); // Use the shader program stored in m_ID
}
/**
 * Destructor for AShader class
 * Cleans up the shader program by deleting it from OpenGL
 */
AShader::~AShader()
{
    glDeleteProgram(m_ID); // Delete the shader program from GPU memory
}