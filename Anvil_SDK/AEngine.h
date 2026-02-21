#pragma once
#define NOMINMAX
#include "ACore.h"
#include "AMath.h"
#include "AShader.h"
#include "AnvilBSPFormat.h"
#include "AnvilPhysics.h"
#include "AResourceManager.h"
#include <Windows.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <string>
#include <vector>
#include "AEntity.h"
#include <functional>
#include <map>

class IGame;

/**
 * @class AEngine
 * @brief The main engine class that manages the application lifecycle, rendering, entities, and game logic.
 */
class ANVIL_API AEngine
{
  public:
    /**
     * @brief Constructor for AEngine
     */
    AEngine();
    /**
     * @brief Virtual destructor for AEngine
     */
    virtual ~AEngine();

    /**
     * @brief Main engine loop that runs the application
     */
    void     Run();
    /**
     * @brief Loads a map by name
     * @param mapName The name of the map to load
     */
    void     LoadMap(const char* mapName);
    /**
     * @brief Creates a new entity with the specified name
     * @param name The name for the new entity
     * @return Pointer to the created entity
     */
    AEntity* CreateEntity(std::string name);
    /**
     * @brief Binds an action to a trigger name
     * @param name The trigger name to bind to
     * @param action The function to execute when the trigger is activated
     */
    void     BindAction(std::string name, std::function<void()> action)
    {
        m_triggerCallbacks[name] = action;
    }
    /**
     * @brief Executes the action associated with the trigger name
     * @param name The trigger name to activate
     */
    void OnTrigger(std::string name)
    {
        if (m_triggerCallbacks.count(name))
			m_triggerCallbacks[name]();
    }
    /**
     * @brief Gets the singleton instance of the engine
     * @return Pointer to the engine instance
     */
    static AEngine* Get()
    {
        return s_Instance;
    }
    /**
     * @brief Gets the physics world instance
     * @return Pointer to the physics world
     */
    AnvilPhysics* GetPhysics()
    {
        return m_physicsWorld;
    }

  private:
    static AEngine* s_Instance;  // Singleton instance of the engine

    AnvilPhysics*                                m_physicsWorld    = nullptr;  // Physics world instance
    AShader*                                     m_mainShader      = nullptr;  // Main shader program
    IGame*                                       m_game            = nullptr;  // Game interface implementation
    HMODULE                                      m_gameLib         = nullptr;  // Game library module handle
    GLFWwindow*                                  m_window          = nullptr;  // GLFW window instance
    AResourceManager*                            m_resourceManager = nullptr;  // Resource manager for assets
    std::vector<AEntity*>                        m_entities;       // Collection of all entities in the scene
    std::vector<AVertex>                         m_worldVerts;     // Vertices for the world geometry
    std::vector<AFace>                           m_worldFaces;     // Faces for the world geometry
    uint32_t                                     m_worldVAO = 0, m_worldVBO = 0, m_worldEBO = 0;  // VAO, VBO, and EBO for world geometry
    uint32_t                                     m_worldIndexCount = 0;  // Number of indices in the world geometry
    float                                        m_lastFrameTime   = 0.0f;  // Time of the last frame for delta time calculation
    float                                        m_deltaTime       = 0.0f;
    std::map<std::string, std::function<void()>> m_triggerCallbacks;  // Map of trigger names to callback functions
};