#include "AnvilPhysics.h"
#include "AEngine.h"
#include <iostream>
#include <print>
#include <set>
#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

struct ATriggerVolume
{
    btGhostObject* ghost;
    std::string    name;
    std::set<const btCollisionObject*> lastOverlap;
};
btVector3 AnvilPhysics::toBullet(const glm::vec3& v)
{
    return btVector3(v.x, v.y, v.z);
}

glm::vec3 AnvilPhysics::toGlm(const btVector3& v)
{
    return glm::vec3(v.x(), v.y(), v.z());
}

/**
 * Constructor for AnvilPhysics class
 * Initializes the physics engine
 */
AnvilPhysics::AnvilPhysics()
{
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher             = new btCollisionDispatcher(m_collisionConfiguration);
    m_broadphase             = new btDbvtBroadphase();
    m_solver                 = new btSequentialImpulseConstraintSolver();
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
    // 25 units is better than -9.81f
    m_dynamicsWorld->setGravity(btVector3(0, -25, 0));
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
        // Check if the body has a corresponding bullet rigid body
        if (body->bulletBody)
        {
            btRigidBody* rb = static_cast<btRigidBody*>(body->bulletBody);
            m_dynamicsWorld->removeRigidBody(rb);
            delete rb->getMotionState();
            delete rb;
        }
        // Delete the body object
        delete body;
    }
    if (m_worldBody)
    {
        m_dynamicsWorld->removeRigidBody(m_worldBody);
        delete m_worldBody->getMotionState();
        delete m_worldBody;
    }
    delete m_meshShape;
    delete m_triangleMesh;

    for (auto* t : m_triggers)
    {
        m_dynamicsWorld->removeCollisionObject(t->ghost);
        delete t->ghost;
        delete t;
    }
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_broadphase;
    delete m_dispatcher;
    delete m_collisionConfiguration;
}

ABody* AnvilPhysics::CreateBody(glm::vec3 pos, glm::vec3 size, float mass, bool isStatic, ECollisionQuality quality, AMesh* mesh)
{
    btVector3   halfExtents = toBullet(size) * 0.5f;
    btCollisionShape* shape       = nullptr;
    switch (quality) {
    case ECollisionQuality::PERFOMANCE:
    {
        btBoxShape* boxShape = new btBoxShape(halfExtents);
        boxShape->setMargin(0.04f);
        shape = boxShape;
        break;
    
    }
		case ECollisionQuality::BALANCED:
        {
            btBoxShape* boxShape = new btBoxShape(halfExtents);
            boxShape->setMargin(0.01f);
			shape = boxShape;
            break;
        }
		case ECollisionQuality::HIGH_FIDELITY:
            if (mesh)
            {
                const std::vector<MVertex>& verts = mesh->GetVertices();
                if (verts.size() >= 4)
                {
                    btConvexHullShape* hull = new btConvexHullShape();
                    for (const auto& v : verts)
                        hull->addPoint(toBullet(v.pos));
                    hull->setMargin(0.01f);                
                    shape=hull;
                }
                else
                {
                btBoxShape* boxShape = new btBoxShape(halfExtents);
				boxShape->setMargin(0.01f);
				shape = boxShape;
                }
            }
            else
            {
                btBoxShape* boxShape = new btBoxShape(halfExtents);
				boxShape->setMargin(0.01f);
				shape = boxShape;
            }
            break;
            
	}

    btTransform startTransform;
	startTransform.setIdentity();
    startTransform.setOrigin(toBullet(pos));

    btScalar btMass = isStatic ? 0 : mass;
    btVector3 localInertia(0, 0, 0);
    if (!isStatic)
        shape->calculateLocalInertia(btMass, localInertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(btMass, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    if (isStatic)
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

    m_dynamicsWorld->addRigidBody(body);

    ABody* aBody       = new ABody;
    aBody->bulletBody    = body;
    aBody->position    = pos;
    aBody->velocity    = glm::vec3(0);
    aBody->halfSize    = size * 0.5f;
    aBody->mass        = mass;
    aBody->orientation = glm::quat(1.0f, 0, 0, 0);
    aBody->isStatic    = isStatic;
    aBody->onGround    = false;
    aBody->collider    = shape;
    aBody->quality      = quality;

    body->setUserPointer(aBody);
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
    // Build triangle mesh
    m_triangleMesh = new btTriangleMesh();
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
            int i0 = f.firstVertex;
            int i1 = f.firstVertex + i;
            int i2 = f.firstVertex + i + 1;
            // Add triangle vertices to IBO
            m_ibo.push_back(i0);      // First vertex of the face
            m_ibo.push_back(i1);
            m_ibo.push_back(i2);
            const btVector3 v0(m_vbo[i0*3], m_vbo[i0*3+1], m_vbo[i0*3+2]);
            const btVector3 v1(m_vbo[i1*3], m_vbo[i1*3+1], m_vbo[i1*3+2]);
            const btVector3 v2(m_vbo[i2*3], m_vbo[i2*3+1], m_vbo[i2*3+2]);
            m_triangleMesh->addTriangle(v0, v1, v2);
       }
    }

    m_meshShape = new btBvhTriangleMeshShape(m_triangleMesh, true);
    btTransform startTransform;
    startTransform.setIdentity();
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, m_meshShape, btVector3(0,0,0));
    m_worldBody = new btRigidBody(rbInfo);
    m_worldBody->setCollisionFlags(m_worldBody->getCollisionFlags() |
                                   btCollisionObject::CF_STATIC_OBJECT);
    m_dynamicsWorld->addRigidBody(m_worldBody);

    std::cout << "[Anvil] World Physics Mesh Baked." << std::endl; // not baked lmao
}
void AnvilPhysics::Update(float dt)
{
    m_dynamicsWorld->stepSimulation(dt, 10);
    for (auto* aBody : m_bodies)
    {
        btRigidBody* rb = static_cast<btRigidBody*>(aBody->bulletBody);
        if (!rb)
            continue;
        btTransform trans;
        if (rb->getMotionState())
            rb->getMotionState()->getWorldTransform(trans);
        else
            trans = rb->getWorldTransform();

        aBody->position    = toGlm(trans.getOrigin());
        aBody->orientation = glm::quat(trans.getRotation().w(), trans.getRotation().x(),
                                       trans.getRotation().y(), trans.getRotation().z());
        aBody->velocity    = toGlm(rb->getLinearVelocity());
        aBody->onGround    = IsGrounded(aBody);
    }
    for (auto* t : m_triggers)
    {
        btGhostObject*                     ghost = t->ghost;
        std::set<const btCollisionObject*> currentOverlap;

        int numOverlap = ghost->getNumOverlappingObjects();
        for (int i = 0; i < numOverlap; i++)
        {

            const btCollisionObject* obj = ghost->getOverlappingObject(i);
            currentOverlap.insert(obj);

            if (t->lastOverlap.find(obj) == t->lastOverlap.end())
            {
                if (obj->getUserPointer())
                {
                    OnTrigger(t->name);
                }
            }
        }
        t->lastOverlap = currentOverlap;
    }
}

void AnvilPhysics::OnTrigger(const std::string& name)
{
    AEngine::Get()->OnTrigger(name);
}
void AnvilPhysics::SetBodyMaterial(ABody* body, float bounciness, float friction)
{
    if (!body || !body->bulletBody)
        return;
    btRigidBody* rb = static_cast<btRigidBody*>(body->bulletBody);
    rb->setRestitution(bounciness);
    rb->setFriction(friction);
    
}
/**
 * Checks if a given body is grounded by casting a ray downward from its position
 * @param body The body to check if it's grounded
 * @return Returns true if the body is grounded, false otherwise
 */
bool AnvilPhysics::IsGrounded(ABody* body)
{
    // Safety first
    if (!body)
        return false;
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
    btGhostObject* ghost = new btGhostObject();
    btBoxShape*    boxShape = new btBoxShape(toBullet(size * 0.5f));
    ghost->setCollisionShape(boxShape);
    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
    btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(toBullet(pos));
    ghost->setWorldTransform(startTransform);

    m_dynamicsWorld->addCollisionObject(ghost);

    ATriggerVolume* trigger = new ATriggerVolume;
	trigger->ghost          = ghost;
	trigger->name           = name;
	m_triggers.push_back(trigger);

    std::cout << "[Anvil] Trigger '" << name << "' registered at " << pos.x << ", " << pos.y << ", "
              << pos.z << std::endl;
}

RaycastHit AnvilPhysics::CastRay(glm::vec3 origin, glm::vec3 direction, float maxDistance,
                                 const std::vector<AEntity*>& entities)
{
    btVector3 from = toBullet(origin);
    btVector3 to   = toBullet(origin + direction * maxDistance);

    btCollisionWorld::ClosestRayResultCallback callback(from, to);

    m_dynamicsWorld->rayTest(from, to, callback);

    RaycastHit hit;
    hit.hit = callback.hasHit();

    if (hit.hit)
    {
        hit.point    = toGlm(callback.m_hitPointWorld);
        hit.normal   = toGlm(callback.m_hitNormalWorld);
        hit.distance = maxDistance * callback.m_closestHitFraction;

        if (callback.m_collisionObject && callback.m_collisionObject->getUserPointer())
        {
            ABody* aBody = static_cast<ABody*>(callback.m_collisionObject->getUserPointer());
        }
    }
    else
    {
        hit.distance = maxDistance;
    }

    return hit;
}
