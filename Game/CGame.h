#pragma once
#include "IGame.h"
#include "AMath.h"
#include "AnvilCamera.h"
#include <vector>

class AEntity;
class RigidBodyComponent;

class CGame : public IGame
{
public:
    CGame() = default;
    virtual ~CGame() = default;

    virtual void OnInit(AEngine* engine) override;
    virtual void OnUpdate(float dt) override;
    virtual void OnShutdown() override;

    virtual glm::mat4 GetViewMatrix() override {
        return m_camera ? m_camera->GetViewMatrix() : glm::mat4(1.0f);
    }

private:
    AEntity* m_playerEntity = nullptr;
    AEntity* m_crate = nullptr;

    RigidBodyComponent* m_playerRB = nullptr;
    ACamera* m_camera = nullptr;

    bool  m_firstMouse = true;
    float m_lastX = 400.0f;
    float m_lastY = 300.0f;
};
extern "C" __declspec(dllexport) IGame* CreateGame() {
    return new CGame();
}