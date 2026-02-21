#include "AnvilPhysics.h"
#include "AEngine.h"
#include <iostream>
#include <reactphysics3d/reactphysics3d.h>
using namespace reactphysics3d;

class AnvilRaycastCallback : public RaycastCallback
{
  public:
    struct
    {
        bool            hit = false;
        glm::vec3       point;
        glm::vec3       normal;
        float           fraction = 1.0f;
        Collider* collider = nullptr;
    } result;

/**
 * Handles notification of a ray hit event
 * @param info Contains detailed information about the raycast hit
 * @return The hit fraction of the raycast
 */
    virtual decimal notifyRaycastHit(const RaycastInfo& info) override
    {
        result.hit      = true;                                    // Set hit status to true
        result.point    = glm::vec3(info.worldPoint.x, info.worldPoint.y, info.worldPoint.z); // Store the hit point coordinates
        result.normal   = glm::vec3(info.worldNormal.x, info.worldNormal.y, info.worldNormal.z); // Store the surface normal at hit point
        result.fraction = info.hitFraction;                        // Store the hit fraction (distance along ray)
        result.collider = info.collider;                           // Store the collider that was hit
        return info.hitFraction;                                   // Return the hit fraction
    }
};
class MyContactListener : public EventListener
{
  public:
/**
 * Constructor for MyContactListener
 * @param physics Pointer to AnvilPhysics object
 * @param names Reference to unordered_map mapping RigidBody pointers to their names
 */
    MyContactListener(AnvilPhysics* physics, std::unordered_map<RigidBody*, std::string>& names)
        : m_physics(physics), m_triggerNames(names) // Initialize member variables with provided parameters
    {
    }

    /**
     * Handles contact events between colliders in the physics simulation
     * This method is called whenever contact between colliders occurs
     * @param callbackData Contains information about the contact pairs
     */
    virtual void onContact(const CallbackData& callbackData) override
    {
        // Iterate through all contact pairs in the callback data
        for (uint p = 0; p < callbackData.getNbContactPairs(); p++)
        {
            // Get the current contact pair
            ContactPair pair = callbackData.getContactPair(p);

            // Only trigger on the "Start" event (entry)
            // This ensures we only process the initial contact, not ongoing contacts
            if (pair.getEventType() == ContactPair::EventType::ContactStart)
            {
                // Get the colliders involved in the contact
                Collider* c1 = pair.getCollider1();
                Collider* c2 = pair.getCollider2();

                // Check if the first collider is a trigger and handle it if true
                if (c1->getIsTrigger())
                    handleTrigger(c1->getBody());
                // Check if the second collider is a trigger and handle it if true
                if (c2->getIsTrigger())
                    handleTrigger(c2->getBody());
            }
        }
    }

  private:
    AnvilPhysics*                                m_physics;
    std::unordered_map<RigidBody*, std::string>& m_triggerNames;

/**
 * Handles a trigger event for a given physics body
 * @param body Pointer to the physics body that triggered the event
 */
    void handleTrigger(Body* body)
    {
    // Convert the generic Body pointer to a RigidBody pointer
        RigidBody* rb = static_cast<RigidBody*>(body);
    // Check if the rigid body is in the trigger names map
        if (m_triggerNames.count(rb))
        {
        // If found, trigger the corresponding event in the physics system
            m_physics->OnTrigger(m_triggerNames[rb]);
        }
    }
};

/**
 * Constructor for AnvilPhysics class
 * Initializes the physics engine and sets up the world with contact listener
 */
AnvilPhysics::AnvilPhysics()
{
    // Create a new PhysicsCommon instance for physics engine initialization
    m_physicsCommon = new PhysicsCommon();
    // Create the physics world using the PhysicsCommon instance
    m_physicsWorld  = m_physicsCommon->createPhysicsWorld();

    // We pass the map reference so the listener can find trigger names
    m_contactListener = new MyContactListener(this, m_triggerNames);
    // Set the event listener to handle physics-related events
    m_physicsWorld->setEventListener(m_contactListener);
}

/**
 * Destructor for AnvilPhysics class
 * Cleans up all physics-related resources including rigid bodies, 
 * triangle meshes, physics world, and contact listener
 */
AnvilPhysics::~AnvilPhysics()
{
    // Iterate through all bodies and clean them up
    for (auto* body : m_bodies)
    {
        // Check if the body has a corresponding ReactPhysics3D rigid body
        if (body->rp3dBody)
            // Destroy the ReactPhysics3D rigid body
            m_physicsWorld->destroyRigidBody(static_cast<RigidBody*>(body->rp3dBody));
        // Delete the body object
        delete body;
    }

    // Clean up the world body if it exists
    if (m_worldBody)
        m_physicsWorld->destroyRigidBody(m_worldBody);
    // Clean up the triangle mesh if it exists
    if (m_triangleMesh)
        m_physicsCommon->destroyTriangleMesh(m_triangleMesh);

    // Concave shapes and world are cleaned up by Common/World
    delete m_physicsCommon;
    delete m_contactListener;
}

ABody* AnvilPhysics::CreateBody(glm::vec3 pos, glm::vec3 size, float mass, bool isStatic)
{
    Vector3   halfExtents = toRP3D(size) * 0.5f;
    BoxShape* boxShape    = m_physicsCommon->createBoxShape(halfExtents);

    Transform  transform(toRP3D(pos), Quaternion::identity());
    RigidBody* body = m_physicsWorld->createRigidBody(transform);

    if (isStatic)
    {
        body->setType(BodyType::STATIC);
    }
    else
    {
        body->setType(BodyType::DYNAMIC);
        body->setMass(mass);
        body->setAngularLockAxisFactor(Vector3(0, 0, 0));
    }

    Collider* collider = body->addCollider(boxShape, Transform::identity());
    Material& mat      = collider->getMaterial();
    mat.setBounciness(0.5f);
    mat.setFrictionCoefficient(0.5f);

    ABody* aBody       = new ABody;
    aBody->rp3dBody    = body;
    aBody->position    = pos;
    aBody->velocity    = glm::vec3(0);
    aBody->halfSize    = size * 0.5f;
    aBody->mass        = mass;
    aBody->orientation = glm::quat(1.0f, 0, 0, 0);
    aBody->isStatic    = isStatic;
    aBody->onGround    = false;
    aBody->collider    = collider;

    body->setUserData(aBody);
    m_bodies.push_back(aBody);
    return aBody;
}

/**
 * Sets up the physics world data from vertex and face information
 * @param verts Vector of vertices defining the mesh
 * @param faces Vector of faces defining the mesh connectivity
 */
void AnvilPhysics::SetWorldData(const std::vector<AVertex>& verts, const std::vector<AFace>& faces)
{
    // 1. Flatten data into the class-member vectors (m_vbo, m_ibo)
    // IMPORTANT: Use your header members here so the memory stays alive!
    m_vbo.clear();  // Clear existing vertex buffer data
    m_ibo.clear();  // Clear existing index buffer data

    // Process vertices and add them to the vertex buffer object (VBO)
    for (const auto& v : verts)
    {
        // Push vertex position coordinates (x, y, z) into VBO
        m_vbo.push_back(v.position.x);
        m_vbo.push_back(v.position.y);
        m_vbo.push_back(v.position.z);
    }

    // Process faces and generate triangles for the index buffer object (IBO)
    for (const auto& f : faces)
    {
        // Generate triangles using fan triangulation
        for (uint32_t i = 1; i < f.numVertices - 1; i++)
        {

            // Add triangle vertices to IBO
            m_ibo.push_back(f.firstVertex);      // First vertex of the face
            m_ibo.push_back(f.firstVertex + i);
            m_ibo.push_back(f.firstVertex + i + 1);
        }
    }

    // 2. Create the Triangle Array wrapper
    TriangleVertexArray* triangleArray =
        new TriangleVertexArray((uint32) verts.size(),     // nbVertices
                                m_vbo.data(),              // verticesStart
                                3 * sizeof(float),         // verticesStride
                                (uint32) m_ibo.size() / 3, // nbTriangles
                                m_ibo.data(),              // indexesStart
                                3 * sizeof(int),           // indexesStride
                                TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
                                TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

    // 3. Create the mesh by passing the array to the factory (Fixes E0165 & E0135)
    // Note: RP3D takes a collection of arrays, so we pass triangleArray by address
    std::vector<Message> messages;
    m_triangleMesh = m_physicsCommon->createTriangleMesh(*triangleArray, messages);
    // 4. Create the shape and static body
    m_meshShape = m_physicsCommon->createConcaveMeshShape(m_triangleMesh);
    m_worldBody = m_physicsWorld->createRigidBody(Transform::identity());
    m_worldBody->setType(BodyType::STATIC);
    m_worldBody->addCollider(m_meshShape, Transform::identity());

    std::cout << "[Anvil] World Physics Mesh Baked." << std::endl;
}
void AnvilPhysics::Update(float dt)
{
    m_physicsWorld->update(dt);

    for (auto* aBody : m_bodies)
    {
        RigidBody* rb   = static_cast<RigidBody*>(aBody->rp3dBody);
        rp3d::Transform transform = rb->getTransform();
        aBody->position = toGlm(transform.getPosition());
        aBody->velocity = toGlm(rb->getLinearVelocity());
        rp3d::Quaternion q = transform.getOrientation();
        // toGlm doesn't handle quats, only vec3
        aBody->orientation = glm::quat(q.w, q.x, q.y, q.z);
        // Simple heuristic: if we aren't moving vertically, we might be grounded
        aBody->onGround = IsGrounded(aBody);
    }
}

// Helper Conversions
Vector3 AnvilPhysics::toRP3D(const glm::vec3& v)
{
    return Vector3(v.x, v.y, v.z);
}
glm::vec3 AnvilPhysics::toGlm(const Vector3& v)
{
    return glm::vec3(v.x, v.y, v.z);
}
void AnvilPhysics::OnTrigger(const std::string& name)
{
    AEngine::Get()->OnTrigger(name);
}
void AnvilPhysics::SetBodyMaterial(ABody* body, float bounciness, float friction)
{
    if (!body || !body->rp3dBody)
        return;
    RigidBody* rb = static_cast<RigidBody*>(body->rp3dBody);
    if (rb->getNbColliders() > 0)
    {
        Collider* collider = rb->getCollider(0);
        Material& material = collider->getMaterial();
        material.setBounciness(bounciness);
        material.setFrictionCoefficient(friction);
    }
    
}
/**
 * Checks if a given body is grounded by casting a ray downward from its position
 * @param body The body to check if it's grounded
 * @return Returns true if the body is grounded, false otherwise
 */
bool AnvilPhysics::IsGrounded(ABody* body)
{
    // Set the starting position for the ray cast to the body's current position
    glm::vec3 start = body->position;
    // Define the direction of the ray (straight downward)
    glm::vec3 direction(0, -1, 0);

    // Calculate the ray length to be slightly more than the body's half height to ensure detection
    float rayLength = body->halfSize.y + 0.2f;
    // Cast a ray downward and store the result in hit
    RaycastHit hit       = CastRay(start, direction, rayLength, {});
    // Return whether the ray hit anything (indicating the body is grounded)
	return hit.hit;
}
void AnvilPhysics::AddTrigger(glm::vec3 pos, glm::vec3 size, std::string name)
{
    rp3d::Vector3   halfExtents = toRP3D(size) * 0.5f;
    rp3d::BoxShape* boxShape    = m_physicsCommon->createBoxShape(halfExtents);

    rp3d::Transform transform(toRP3D(pos), rp3d::Quaternion::identity());

    rp3d::RigidBody* body = m_physicsWorld->createRigidBody(transform);
    body->setType(rp3d::BodyType::STATIC);

    rp3d::Collider* collider = body->addCollider(boxShape, rp3d::Transform::identity());
    collider->setIsTrigger(true);

    m_triggerNames[body] = name;

    std::cout << "[Anvil] Trigger '" << name << "' registered at " << pos.x << ", " << pos.y << ", "
              << pos.z << std::endl;
}
RaycastHit AnvilPhysics::CastRay(glm::vec3 origin, glm::vec3 direction, float maxDistance,
                                 const std::vector<AEntity*>& entities)
{
    glm::vec3            target = origin + (direction * maxDistance);
    rp3d::Ray            ray(toRP3D(origin), toRP3D(target));
    AnvilRaycastCallback callback;

    // Perform the raycast in the world
    m_physicsWorld->raycast(ray, &callback);

    RaycastHit finalHit;
    finalHit.hit = callback.result.hit;

    if (finalHit.hit)
    {
        finalHit.point    = callback.result.point;
        finalHit.distance = maxDistance * callback.result.fraction;

        // We look at the body's UserData to see which Entity we hit
        if (callback.result.collider)
        {
            auto* baseBody  = callback.result.collider->getBody();
            auto* rp3dBody  = static_cast<rp3d::RigidBody*>(baseBody);
            finalHit.entity = (AEntity*) rp3dBody->getUserData();
        }
    }

    return finalHit;
}
