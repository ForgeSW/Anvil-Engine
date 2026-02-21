// APhysicsWorld.cpp
#include "APhysicsWorld.h"
#include <reactphysics3d/reactphysics3d.h>

struct APhysicsWorld::PhysicsData
{
    rp3d::PhysicsCommon physicsCommon;
	rp3d::PhysicsWorld* world;
};
APhysicsWorld::APhysicsWorld()
{
    m_data = new PhysicsData();
    m_data->world = m_data->physicsCommon.createPhysicsWorld();
}

APhysicsWorld::~APhysicsWorld()
{
    m_data->physicsCommon.destroyPhysicsWorld(m_data->world);
    delete m_data;
}

void APhysicsWorld::Update(float deltaTime)
{
    m_data->world->update(deltaTime);
}

void* APhysicsWorld::CreateBox(glm::vec3 pos, glm::vec3 halfExtents, bool isStatic)
{
    rp3d::Vector3    position(pos.x, pos.y, pos.z);
    rp3d::Quaternion orientation = rp3d::Quaternion::identity();
    rp3d::Transform  transform(position, orientation);

    rp3d::RigidBody* body = m_data->world->createRigidBody(transform);
    body->setType(isStatic ? rp3d::BodyType::STATIC : rp3d::BodyType::DYNAMIC);

    const rp3d::Vector3 extent(halfExtents.x, halfExtents.y, halfExtents.z);
    rp3d::BoxShape*     shape = m_data->physicsCommon.createBoxShape(extent);

    body->addCollider(shape, rp3d::Transform::identity());

    return static_cast<void*>(body);
}