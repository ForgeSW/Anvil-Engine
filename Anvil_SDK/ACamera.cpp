#include "AnvilCamera.h"

/**
 * Constructor for ACamera class
 * Initializes camera with position and default vectors
 * @param position The initial position of the camera in 3D space
 */
ACamera::ACamera(glm::vec3 position) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Yaw(-90.0f), Pitch(0.0f)
{
    Position = position;  // Set the camera position
    WorldUp  = glm::vec3(0.0f, 1.0f, 0.0f);  // Define world up vector
    updateCameraVectors();  // Update camera vectors based on initial values
}

glm::mat4 ACamera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

/**
 * Processes keyboard input to move the camera in different directions
 * @param direction The direction of camera movement (FORWARD, BACKWARD, LEFT, RIGHT)
 * @param deltaTime The time elapsed since the last frame, used to calculate movement distance
 */
void ACamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    // Calculate movement velocity based on speed and time elapsed
    float velocity = MovementSpeed * deltaTime;
    // Move forward
    if (direction == FORWARD)
        Position += Front * velocity;
    // Move backward
    if (direction == BACKWARD)
        Position -= Front * velocity;
    // Move left
    if (direction == LEFT)
        Position -= Right * velocity;
    // Move right
    if (direction == RIGHT)
        Position += Right * velocity;
}

/**
 * Processes mouse movement input and updates camera orientation
 * @param xoffset Horizontal offset of mouse movement
 * @param yoffset Vertical offset of mouse movement
 */
void ACamera::ProcessMouseMovement(float xoffset, float yoffset)
{
    // Apply mouse sensitivity to the input offsets
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    // Update yaw and pitch based on mouse movement
    Yaw += xoffset;
    Pitch += yoffset;

    // Constrain pitch to avoid flipping the camera
    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;

    // Update camera vectors based on new yaw and pitch values
    updateCameraVectors();
}

/**
 * Updates the camera's vectors based on its current yaw and pitch values.
 * This function recalculates the front vector, right vector, and up vector
 * to maintain the correct orientation of the camera in 3D space.
 */
void ACamera::updateCameraVectors()
{
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)); // Calculate x component using yaw and pitch
    front.y = sin(glm::radians(Pitch));                          // Calculate y component using pitch
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)); // Calculate z component using yaw and pitch
    // Normalize the front vector and update the camera's front direction
    Front   = glm::normalize(front);
    // Calculate the right vector using cross product of front and world up vector
    Right   = glm::normalize(glm::cross(Front, WorldUp));
    // Recalculate the up vector using cross product of right and front vectors
    Up      = glm::normalize(glm::cross(Right, Front));
}