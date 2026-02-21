#pragma once
#include <glad/glad.h>
#include "AEntity.h"
#include "AMesh.h"
#include "AShader.h"
#include "IComponent.h"
#include "AMath.h"

class ANVIL_API MeshComponent : public IComponent
{
  public:
    MeshComponent(AMesh* mesh) : m_mesh(mesh)
    {
    }
/**
 * Initialize the entity component with its owner
 * @param owner Pointer to the AEntity that owns this component
 */
    void OnInit(AEntity* owner) override
    {
        m_owner = owner; // Store the owner entity pointer
    }
    /**
     * Called once per frame to update the game state
     * @param dt The time elapsed since the last update in seconds
     */
    void OnUpdate(float dt) override
    {
        // This is a placeholder implementation
        // The actual update logic should be implemented in derived classes
    }
/**
 * Renders the mesh component using the specified shader
 * @param shader The shader program to use for rendering
 */
    void OnRender(AShader* shader) override
    {
    // Early return if mesh or owner is not valid
        if (!m_mesh || !m_owner)
            return;
    // Initialize model matrix as identity matrix
        glm::mat4 model = glm::mat4(1.0f);

    
    // Apply transformations in order: translation, rotation, scaling
        model           = glm::translate(model, m_owner->position);           // Move to owner's position
        model           = glm::rotate(model, glm::radians(m_owner->rotation.y), {0, 1, 0}); // Rotate around Y-axis
        model           = glm::scale(model, m_owner->scale);                  // Apply scaling

    // Set the model matrix uniform in the shader
        glUniformMatrix4fv(glGetUniformLocation(shader->GetID(), "model"), 1, GL_FALSE,
    // Draw the mesh
                           glm::value_ptr(model));
        m_mesh->Draw();
    }

  private:
    AMesh* m_mesh;
};