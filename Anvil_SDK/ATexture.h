#pragma once
#include <glad/glad.h>
#include <stb_image.h>
#include <string>

class ATexture
{
  public:
    unsigned int ID;
    std::string  type;
    std::string  path;

/**
 * Constructor for ATexture class that loads an image from a file path and creates an OpenGL texture
 * @param path The file path to the image texture
 */
    ATexture(const char* path)
    {
    // Generate a texture name and store it in ID
        glGenTextures(1, &ID);



    // Load image data from file using stb_image library
        int width, height, nrComponents;  // Variables to store image dimensions and components
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
        // Determine texture format based on number of components (RGB or RGBA)
            GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;

        // Bind the texture to the GL_TEXTURE_2D target
            glBindTexture(GL_TEXTURE_2D, ID);
        // Create and initialize the texture image
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE,
                         data);
        // Generate mipmaps for the texture
            glGenerateMipmap(GL_TEXTURE_2D);




        // Set texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Set wrapping to repeat on the S axis
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Set wrapping to repeat on the T axis

        
        // Set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // Set minification filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // Set magnification filter

        // Free the loaded image data
            stbi_image_free(data);
        }
    }

/**
 * Binds the texture to a specific texture unit for rendering
 * @param unit The texture unit to bind to (default is 0)
 */
    void Bind(unsigned int unit = 0)
    {
    // Activate the specified texture unit
        glActiveTexture(GL_TEXTURE0 + unit);
    // Bind the texture to the currently active texture unit
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};