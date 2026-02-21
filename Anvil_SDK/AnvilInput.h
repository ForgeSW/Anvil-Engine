#pragma once
#include <glfw/glfw3.h>
#include <map>

/**
 * @class AnvilInput
 * @brief A static class for handling keyboard input using GLFW.
 */
class AnvilInput
{
  public:
    /**
     * @brief Checks if a specific key is currently pressed.
     * @param key The key to check (using GLFW key codes).
     * @return True if the key is pressed, false otherwise.
     */
    static bool IsKeyPressed(int key)
    {
        auto window = glfwGetCurrentContext(); // Get the current GLFW window context
        return glfwGetKey(window, key) == GLFW_PRESS; // Check if the key is in the pressed state
    }
};