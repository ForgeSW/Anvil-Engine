#pragma once
// APhysicsWorld.h
#include "ACore.h"
#include "AMath.h"


class ANVIL_API APhysicsWorld
{
  public:
    APhysicsWorld();
    ~APhysicsWorld();

    void             Update(float deltaTime);
    void* CreateBox(glm::vec3 pos, glm::vec3 halfExtents, bool isStatic = false);

  private:
    struct PhysicsData;
    PhysicsData* m_data;
};