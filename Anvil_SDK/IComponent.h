#pragma once
#include "ACore.h"

class AEntity;
class AShader;

/**
 * @brief The IComponent class is an abstract base class that defines the interface for all components in the ANVIL engine.
 * Components are modular parts that can be attached to entities to give them specific functionality.
 */
class ANVIL_API IComponent
{
  public:
    /**
     * @brief Virtual destructor for the IComponent class.
     * Ensures proper cleanup of derived class objects when deleted through a base class pointer.
     */
    virtual ~IComponent()
    {
    }
    /**
     * @brief Called when the component is initialized.
     * @param owner A pointer to the AEntity that owns this component.
     */
    virtual void OnInit(AEntity* owner)    = 0;
    /**
     * @brief Called once per frame to update the component's state.
     * @param dt The time elapsed since the last frame, in seconds.
     */
    virtual void OnUpdate(float dt)        = 0;
    /**
     * @brief Called to render the component.
     * @param shader The shader program to use for rendering.
     */
    virtual void OnRender(AShader* shader) = 0;
    /**
     * @brief Pointer to the entity that owns this component.
     * This allows the component to access its parent entity and other components attached to it.
     */
    AEntity* m_owner = nullptr;
};