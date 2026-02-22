#include "RigidBodyComponent.h"
#include "AEngine.h"
#include <reactphysics3d/reactphysics3d.h>

/**
 * Initialize the RigidBodyComponent
 * @param owner Pointer to the entity that owns this component
 */
void RigidBodyComponent::OnInit(AEntity* owner)
{
    // Store the owner entity
	m_owner = owner;

    
    // Create a physics body using the engine's physics system
    m_body = AEngine::Get()->GetPhysics()->CreateBody(m_owner->position, m_size, m_mass, m_isStatic, m_quality);

    if (!m_body || !m_body->rp3dBody)
            return;
    
        auto* rb = static_cast<rp3d::RigidBody*>(m_body->rp3dBody);
		rb->setUserData(m_owner);
    
}

/**
 * Update method for the RigidBodyComponent, called each frame
 * @param dt Delta time since last update, used for time-based calculations
 */
void RigidBodyComponent::OnUpdate(float dt)
{
    // Check if the rigid body exists or if the object is static
    // If either condition is true, skip the update
    if (!m_body || m_isStatic)
        return;

    // AnvilPhysics already handles this
    m_owner->position = m_body->position;
}

/**
 * Applies a push force to the rigid body
 * @param force The force vector to be applied to the rigid body
 */
void RigidBodyComponent::ApplyPush(glm::vec3 force)
{
    // Check if the rigid body exists
    if (m_body)
    {
        // Cast the body to rp3d::RigidBody type
        auto* rb = static_cast<rp3d::RigidBody*>(m_body->rp3dBody);
        // Apply the force at the center of mass of the rigid body
        // Convert the force vector from AnvilPhysics format to rp3d format before applying
        rb->applyWorldForceAtCenterOfMass(AnvilPhysics::toRP3D(force));
    }
}

/**
 * Apply torque to the rigid body
 * @param torque The torque vector to apply in world coordinates
 */
void RigidBodyComponent::ApplyTorque(glm::vec3 torque)
{
	// Check if the rigid body exists
	if (m_body)
	{
		// Cast the generic physics body to RP3D rigid body
		auto* rb = static_cast<rp3d::RigidBody*>(m_body->rp3dBody);
        // Ensure the body is awake (not sleeping)
        rb->setIsSleeping(false);
        // Apply the torque in world coordinates
        // Convert the torque vector from AnvilPhysics to RP3D format before applying
        rb->applyWorldTorque(AnvilPhysics::toRP3D(torque));
	}
}

/**
 * Checks if the rigid body is currently grounded
 * 
 * @return true if the rigid body is on the ground, false otherwise
 */
bool RigidBodyComponent::IsGrounded()
{
    return m_body->onGround;  // Return the onGround state of the physics body
}

/**
 * Sets the bounciness of the rigid body
 * @param bounciness A value between 0 and 1 that determines how bouncy the object is
 *                   0 means no bounce, 1 means perfect bounce
 */
void RigidBodyComponent::SetBouciness(float bounciness)
{
    // Set the material properties of the physics body
    // Using default friction value of 0.5f
    AEngine::Get()->GetPhysics()->SetBodyMaterial(m_body, bounciness, 0.5f);
}
