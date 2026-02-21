#pragma once
#include "AMath.h"
#include "IComponent.h"
#include <string>
#include <vector>

/**
 * @class AEntity
 * @brief Represents an entity in the game engine with position, rotation, scale and components
 */
/**
 * @class AEntity
 * @brief Represents a basic entity in the game engine, which can hold components and has transform properties.
 */
class ANVIL_API AEntity
{
  public:

    // Basic transform properties of the entity
    std::string name;      // Name of the entity
    glm::vec3   position = glm::vec3(0.0f); // Position in 3D space
    glm::vec3   rotation = glm::vec3(0.0f); // Rotation in Euler angles
    glm::vec3   scale    = glm::vec3(1.0f); // Scale factor in each axis

    /**
     * @brief Adds a component to the entity and initializes it
     * @param comp Pointer to the component to be added
     */
    void AddComponent(IComponent* comp)
    {

        comp->OnInit(this); // Initialize the component with this entity
        m_components.push_back(comp); // Add component to the component list
    }

    /**
     * @brief Gets a specific component by type
     * @tparam T Type of the component to retrieve
     * @return Pointer to the component if found, nullptr otherwise
     */
    template <typename T> T* GetComponent()
    {
        for (auto* c : m_components)
        {
            T* target = dynamic_cast<T*>(c); // Try to cast component to desired type
            if (target)
                return target; // Return component if cast was successful
        }
        return nullptr; // Return nullptr if component of desired type not found
    }

    /**
     * @brief Gets all components attached to this entity
     * @return Constant reference to the vector of components
     */
    const std::vector<IComponent*>& GetComponents() const
    {
        return m_components;
    }

    /**
     * @brief Updates all components attached to this entity
     * @param dt Delta time for frame-independent updates
     */
    void Update(float dt)
    {
        for (auto* c : m_components)
            c->OnUpdate(dt); // Update each component with delta time
    }

    /**
     * @brief Destructor that cleans up all attached components
     */
    ~AEntity()
    {
        for (auto* c : m_components)
            delete c; // Delete each component to prevent memory leaks
    }

  private:
    std::vector<IComponent*> m_components; // Container for all attached components
};