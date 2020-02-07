#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"

#include <entt/entt.hpp>
#include <raylib.h>
#include <rlgl.h>

using namespace boyd;

/// TODO: add state transfer
struct BoydGfxState
{
    bool isCursorLocked;
};

inline BoydGfxState *GetState(void *state)
{
    return reinterpret_cast<BoydGfxState *>(state);
}

void FlipCursorGrabbing(BoydGfxState *state)
{
    state->isCursorLocked ^= true;
}

void SetCursor(BoydGfxState *state)
{
    if(state->isCursorLocked)
        DisableCursor();
    else
        EnableCursor();
}

inline static ::Matrix Glm2RLMat4(const glm::mat4 &mat)
{
    return *reinterpret_cast<const ::Matrix *>(&mat);
}

extern "C" {

BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    auto *gfxState = new BoydGfxState;

#ifdef DEBUG
    gfxState->isCursorLocked = false;
#else
    gfxState->isCursorLocked = true;
#endif
    return gfxState;
}

BOYD_API void BoydUpdate_Gfx(void *state)
{
    GameState *gameState = Boyd_GameState();
    auto *gfxState = GetState(state);

    glm::vec2 screenSize{GetScreenWidth(), GetScreenHeight()};

    // TODO: Need a way to actually pick what camera to use - find the camera tagged "MainCamera"?
    auto cameraView = gameState->ecs.view<comp::Camera, comp::ActiveCamera>();
    if(cameraView.empty())
    {
        // Hard to render anything without a camera...
        return;
    }
    auto cameraEntity = *cameraView.begin();
    const auto &camera = gameState->ecs.get<comp::Camera>(cameraEntity);

    glm::mat4 projMtx;
    switch(camera.mode)
    {
    case comp::Camera::Persp:
        projMtx = glm::perspectiveFov(camera.fov, screenSize.x, screenSize.y, camera.zNear, camera.zFar);
        break;
    default: // comp::Camera::Ortho
        if(glm::isinf(camera.zNear) || glm::isinf(camera.zFar))
        {
            projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top);
        }
        else
        {
            projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top, camera.zNear, camera.zFar);
        }
        break;
    }

    glm::mat4 viewMtx = glm::identity<glm::mat4>();
    if(gameState->ecs.has<comp::Transform>(cameraEntity))
    {
        viewMtx = gameState->ecs.get<comp::Transform>(cameraEntity).matrix;
        viewMtx = glm::inverse(viewMtx); // Inverse, because this is a camera view matrix!
    }

    const comp::Skybox *skybox = nullptr;
    if(gameState->ecs.has<comp::Skybox>(cameraEntity))
    {
        skybox = &gameState->ecs.get<comp::Skybox>(cameraEntity);
    }

    // -------------------------------------------------------------------------

    ::BeginDrawing();
    ::ClearBackground(BLACK);

    // --- Raylib+glm setup code - based on BeginMode3D() ----------------------
    rlMatrixMode(RL_PROJECTION); // Save the current 2D projection matrix so that `::EndMode3D()` can restore it later
    rlPushMatrix();

    rlglDraw();
    SetMatrixProjection(Glm2RLMat4(projMtx));
    SetMatrixModelview(Glm2RLMat4(viewMtx));
    rlEnableDepthTest();
    // -------------------------------------------------------------------------

    if(skybox)
    {
        DrawModel(skybox->rlSkyboxModel, (Vector3){0, 0, 0}, 1.0f, WHITE);
    }

    gameState->ecs.view<comp::Transform, comp::Mesh>()
        .each([state](auto entity, const auto &transform, auto &mesh) {
            memcpy(&mesh.model.transform, &transform.matrix, sizeof(::Matrix));
            DrawModel(mesh.model, (Vector3){0, 6.0f, 0}, 1.0f, WHITE);
        });

    // -------------------------------------------------------------------------

    ::EndMode3D();
    ::EndDrawing();
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
