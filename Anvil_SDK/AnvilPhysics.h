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
#include <btBulletDynamicsCommon.h>
#include "AMesh.h"

enum class ECollisionQuality
{
    PERFOMANCE,   // Low collisions, fast and uses fewer solver iterations
    BALANCED,     // Medium collisions, the default and best option for most games
    HIGH_FIDELITY // High detailed collisions, unless you wanna burn your cpu don't use this much
};

class AEngine;

// Simple body structure that the game uses
struct ANVIL_API ABody
{
    void*             bulletBody; // Pointer to reactphysics3d::RigidBody, well... btRigidBody now
    btCollisionShape* collider;
    glm::quat         orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3         position; // Updated each frame from rp3d
    glm::vec3         velocity; // Updated each frame from rp3d
    glm::vec3         halfSize; // Half extents (set at creation)
    ECollisionQuality quality;
    float             mass;     // Mass (set at creation)
    bool              isStatic; // Static or dynamic
    bool              onGround; // Computed in Update (via contact info)
};

struct ANVIL_API RaycastHit
{
    bool      hit;
    float     distance;
    glm::vec3 point;
    glm::vec3 normal;
    AEntity*  entity;
};

// Trigger helper struct, defined in cpp
struct ATriggerVolume;

/**
 * @class AnvilPhysics
 * @brief A physics system class using Bullet engine for physics simulation
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
    ABody*     CreateBody(glm::vec3 pos, glm::vec3 size, float mass, bool isStatic,
                          ECollisionQuality quality = ECollisionQuality::BALANCED, AMesh* mesh = nullptr);
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
    static btVector3 toBullet(const glm::vec3& v);
    static glm::vec3 toGlm(const btVector3& v);

  private:
    btDefaultCollisionConfiguration*     m_collisionConfiguration = nullptr;
    btCollisionDispatcher*               m_dispatcher             = nullptr;
    btBroadphaseInterface*               m_broadphase             = nullptr;
    btSequentialImpulseConstraintSolver* m_solver                 = nullptr;
    btDiscreteDynamicsWorld*             m_dynamicsWorld          = nullptr;

    // World static body (mesh)
    btRigidBody*            m_worldBody    = nullptr;
    btTriangleMesh*         m_triangleMesh = nullptr;
    btBvhTriangleMeshShape* m_meshShape    = nullptr;

    // All bodies created (for cleanup & syncing)
    std::vector<ABody*> m_bodies;

    std::vector<ATriggerVolume*> m_triggers;

    std::vector<float> m_vbo;
    std::vector<int>   m_ibo;
};
