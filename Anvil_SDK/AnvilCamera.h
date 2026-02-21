#pragma once
// AnvilCamera.h
#include "ACore.h"
#include "AMath.h"

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class ANVIL_API ACamera
{
  public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed    = 5.0f;
    float MouseSensitivity = 0.1f;

    ACamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

    glm::mat4 GetViewMatrix();
    void      ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void      ProcessMouseMovement(float xoffset, float yoffset);

  private:
    void updateCameraVectors();
};