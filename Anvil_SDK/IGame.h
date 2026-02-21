#pragma once
#include "ACore.h"
#include "AMath.h"

class AEngine;

class IGame
{
  public:
/**
 * Virtual destructor for the IGame interface class.
 * This ensures proper cleanup of derived class objects when deleted through a base class pointer.
 * Virtual destructors are essential in polymorphic classes to prevent resource leaks.
 */
    virtual ~IGame()
    {
    // Empty destructor implementation as the interface doesn't own resources
    }

    /**
     * Initialize the game with the provided engine instance.
     * @param engine Pointer to the engine instance that the game will use for initialization
     */
    virtual void OnInit(AEngine* engine) = 0;

    /**
     * Update the game state with the time elapsed since the last frame.
     * @param dt Time delta in seconds since the last update
     */
    virtual void OnUpdate(float dt) = 0;

    /**
     * Perform cleanup and shutdown operations for the game.
     * This method is called when the game is being terminated.
     */
    virtual void OnShutdown() = 0;

    /**
     * Get the current view matrix for rendering.
     * @return glm::mat4 representing the current view transformation matrix
     */
    virtual glm::mat4 GetViewMatrix() = 0;
};

// Function pointer type for the DLL export
typedef IGame* (*CreateGameFn)();