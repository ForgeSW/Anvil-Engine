#include "CGame.h"
#include <AEngine.h>
#include <AnvilInput.h>
#include <AMeshLoader.h>
#include <MeshComponent.h>
#include <RigidBodyComponent.h>
#include <iostream>
#include <reactphysics3d/reactphysics3d.h>

// Configuration constants
constexpr float SPEED = 12.0f;
constexpr float ACCELERATION = 30.0f;

/**
 * @brief Initialize the game scene with entities, physics, and camera
 * @param engine Pointer to the game engine instance
 */
void CGame::OnInit(AEngine* engine)
{
    // Log initialization message
    std::cout << "[CGame] Initializing Modern Scene..." << std::endl;

    // Load the game map
    engine->LoadMap("world");

    // Create player entity and set its position
    m_playerEntity = engine->CreateEntity("Player");
    m_playerEntity->position = glm::vec3(0, 3.0f, 0);

    // Add RigidBody (Extents, Mass, Static)
    m_playerRB = new RigidBodyComponent(glm::vec3(1, 2, 1), 75.0f, false);
    m_playerEntity->AddComponent(m_playerRB);

    m_camera = new ACamera(glm::vec3(-3, 2, 0));

    AMesh* modelMesh = AMeshLoader::LoadAnvMesh("model.anvmesh");
    if (modelMesh) {
        m_crate = engine->CreateEntity("PhysicsCrate");
        m_crate->position = glm::vec3(0, 5.0f, 0);
        m_crate->AddComponent(new MeshComponent(modelMesh));
        m_crate->AddComponent(new RigidBodyComponent(glm::vec3(1, 1, 1), 60.0f, false));
        m_crate->GetComponent<RigidBodyComponent>()->SetBouciness(0.01f);
    }

    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

/**
 * @brief Update function called every frame to handle game logic and player movement
 * @param dt Delta time since last frame, used for frame-rate independent movement
 */
void CGame::OnUpdate(float dt)
{
    // Early return if essential components are not initialized
    if (!m_playerEntity || !m_camera || !m_playerRB) return;

    // Get the current GLFW window context for input handling
    GLFWwindow* window = glfwGetCurrentContext();

    // Get current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Handle first mouse movement initialization
    if (m_firstMouse) {
        m_lastX = (float)xpos; m_lastY = (float)ypos;
        m_firstMouse = false;
    }

    // Calculate mouse movement offset since last frame
    float xoffset = (float)xpos - m_lastX;
    float yoffset = m_lastY - (float)ypos;
    m_lastX = (float)xpos; m_lastY = (float)ypos;

    // Apply mouse movement to camera
    m_camera->ProcessMouseMovement(xoffset, yoffset);

    // Initialize movement direction vector
    glm::vec3 wishDir(0.0f);
    // Check for movement key presses and update wish direction accordingly
    if (AnvilInput::IsKeyPressed(GLFW_KEY_W)) wishDir += m_camera->Front;
    if (AnvilInput::IsKeyPressed(GLFW_KEY_S)) wishDir -= m_camera->Front;
    if (AnvilInput::IsKeyPressed(GLFW_KEY_A)) wishDir -= m_camera->Right;
    if (AnvilInput::IsKeyPressed(GLFW_KEY_D)) wishDir += m_camera->Right;

    wishDir.y = 0; // Ground plane only - prevent vertical movement with horizontal keys

    // Access the internal physics state
    glm::vec3 currentVel = m_playerRB->m_body->velocity;

    // Apply acceleration if there's a movement direction
    if (glm::length(wishDir) > 1e-4) {
        wishDir = glm::normalize(wishDir);
        currentVel.x = glm::mix(currentVel.x, wishDir.x * SPEED, dt * ACCELERATION);
        currentVel.z = glm::mix(currentVel.z, wishDir.z * SPEED, dt * ACCELERATION);
    }
    else {
        currentVel.x = glm::mix(currentVel.x, 0.0f, dt * ACCELERATION);
        currentVel.z = glm::mix(currentVel.z, 0.0f, dt * ACCELERATION);
    }

    bool grounded = m_playerRB->IsGrounded();
    if (AnvilInput::IsKeyPressed(GLFW_KEY_SPACE) && grounded) {
        currentVel.y = 10.0f;
    }

    // Apply back to the physics engine
    auto* rb = static_cast<rp3d::RigidBody*>(m_playerRB->m_body->rp3dBody);
    rb->setLinearVelocity(rp3d::Vector3(currentVel.x, currentVel.y, currentVel.z));

    // The player position is automatically updated by the RigidBodyComponent's OnUpdate
    m_camera->Position = m_playerEntity->position + glm::vec3(0, 1.7f, 0);
}

/**
 * Shutdown function for the CGame class
 * This function is responsible for cleaning up resources when the game is shutting down
 * Specifically, it handles the deallocation of the camera object
 */
void CGame::OnShutdown() {
    // Check if the camera pointer exists before attempting to delete
    if (m_camera) {
        // Delete the camera object to free memory
        delete m_camera;
        // Set the pointer to nullptr to prevent dangling pointer
        m_camera = nullptr;
    }
}