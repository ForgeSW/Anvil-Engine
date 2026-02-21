#pragma once
#include "ACore.h"
#include "AEntity.h"
#include "AMath.h"
#include "AnvilBSPFormat.h"
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace reactphysics3d
{
class PhysicsCommon;
class PhysicsWorld;
class RigidBody;
class Collider;
class BoxShape;
class TriangleMesh;
class ConcaveMeshShape;
class ContactListener;
class Vector3;
} // namespace reactphysics3d

class AEngine;

// Simple body structure that the game uses
struct ANVIL_API ABody
{
    void* rp3dBody; // Pointer to reactphysics3d::RigidBody    
    reactphysics3d::Collider* collider;
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 position; // Updated each frame from rp3d
    glm::vec3 velocity; // Updated each frame from rp3d
    glm::vec3 halfSize; // Half extents (set at creation)
    
    float     mass;     // Mass (set at creation)

    bool      isStatic; // Static or dynamic
    bool      onGround; // Computed in Update (via contact info)
};

struct ANVIL_API RaycastHit
{
    bool      hit;
    float     distance;
    glm::vec3 point;
    AEntity*  entity;
};

// Trigger volume data (used for naming)
struct ATriggerVolume
{
    glm::vec3   min;
    glm::vec3   max;
    std::string name;
};

// Forward declare our contact listener (defined in .cpp)
class MyContactListener;

/**
 * @class AnvilPhysics
 * @brief A physics system class using ReactPhysics3D engine for physics simulation
 */
class ANVIL_API AnvilPhysics
{
  public:
    /**
     * @brief Constructor for AnvilPhysics
     */
    AnvilPhysics();
    /**
     * @brief Destructor for AnvilPhysics
     */
    ~AnvilPhysics();

    /**
     * @brief Update the physics simulation
     * @param dt Delta time for physics update
     */
    void       Update(float dt);
    ABody*     CreateBody(glm::vec3 pos, glm::vec3 size, float mass, bool isStatic);
    void       SetWorldData(const std::vector<AVertex>& verts, const std::vector<AFace>& faces);
    RaycastHit CastRay(glm::vec3 origin, glm::vec3 direction, float maxDistance,
                       const std::vector<AEntity*>& entities);
    bool       IsGrounded(ABody* body);
    void       AddTrigger(glm::vec3 pos, glm::vec3 size, std::string name);
    void       SetWorldPlanes(const std::vector<APlane>& planes)
    {
    } // Not needed
    void SetWorldBrushes(const std::vector<ABSPBrush>& brushes)
    {
    } // Not needed

    // Called from contact listener when a trigger is entered
    void OnTrigger(const std::string& name);
    void SetBodyMaterial(ABody* body, float bounciness, float friction);
    // Conversion helpers
    static reactphysics3d::Vector3 toRP3D(const glm::vec3& v);
    static glm::vec3               toGlm(const reactphysics3d::Vector3& v);

  private:
    reactphysics3d::PhysicsCommon* m_physicsCommon   = nullptr;
    reactphysics3d::PhysicsWorld*  m_physicsWorld    = nullptr;
    MyContactListener*             m_contactListener = nullptr;

    // World static body (mesh)
    reactphysics3d::RigidBody*        m_worldBody    = nullptr;
    reactphysics3d::TriangleMesh*     m_triangleMesh = nullptr;
    reactphysics3d::ConcaveMeshShape* m_meshShape    = nullptr;

    // All bodies created (for cleanup & syncing)
    std::vector<ABody*> m_bodies;

    // Map rp3d body pointer to trigger name
    std::unordered_map<reactphysics3d::RigidBody*, std::string> m_triggerNames;

    
    std::vector<float>             m_vbo;
    std::vector<int>               m_ibo;
};
