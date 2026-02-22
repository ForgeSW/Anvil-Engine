#pragma once
#include "AMath.h"
#include "IComponent.h"
#include "AnvilPhysics.h"
#include "AEntity.h"
#include "ACore.h"


class AEngine;

class ANVIL_API RigidBodyComponent : public IComponent
{
  public:
    ABody* m_body = nullptr;
    glm::vec3 m_size;
    float     m_mass;
    bool      m_isStatic;
    ECollisionQuality m_quality;

    RigidBodyComponent(glm::vec3 size, float mass = 1.0f, bool isStatic = false, ECollisionQuality quality = ECollisionQuality::BALANCED)
        : m_size(size), m_mass(mass), m_isStatic(isStatic) ,m_quality(quality) {};
    void OnInit(AEntity* owner) override;
    void OnUpdate(float dt) override;
    void OnRender(AShader* shader) override {};

    void ApplyPush(glm::vec3 force);
    void ApplyTorque(glm::vec3 torque);
    bool IsGrounded();
    void SetBouciness(float bounciness);
};