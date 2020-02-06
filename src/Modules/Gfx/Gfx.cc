#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"

#include <entt/entt.hpp>

/// TODO: add state transfer
struct BoydGfxState
{
    bool isCursorLocked;
    bool renderSkybox;
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
extern "C" {
BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    auto *gfxState = new BoydGfxState;

    gfxState->renderSkybox = true;
#ifdef DEBUG
    gfxState->isCursorLocked = false;
#else
    gfxState->isCursorLocked = true;
#endif
    return gfxState;
}

BOYD_API void BoydUpdate_Gfx(void *state)
{
    boyd::GameState *entt_state = Boyd_GameState();
    auto *gfxState = GetState(state);
    Camera *mainCamera;

    entt_state->ecs.view<boyd::comp::Camera>().each([&mainCamera](auto entity, auto &camera) {
        mainCamera = &camera.camera;
    });

#ifdef DEBUG
    if(IsKeyPressed(KEY_U) || (!gfxState->isCursorLocked && IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
    {
        FlipCursorGrabbing(gfxState);
    }

    if(IsKeyPressed(KEY_H))
    {
        BOYD_LOG(Debug, "Flipping renderSkybox");
        gfxState->renderSkybox ^= true;
    }
    SetCursor(gfxState);
#endif

    ClearBackground(BLACK);

    boyd::comp::Skybox *skybox = nullptr;

    entt_state->ecs.view<boyd::comp::Skybox>().each([&skybox](auto entity, auto &skyboxComp) {
        skybox = &skyboxComp;
    });

    // Use Raylib's automatic camera management for us
    ::UpdateCamera(mainCamera);

    ::BeginDrawing();
    ::BeginMode3D(*mainCamera);

    auto rl2glmVec3 = [](const ::Vector3 vec) {
        return glm::vec3{vec.x, vec.y, vec.z};
    };

    auto view = glm::lookAt(rl2glmVec3(mainCamera->position), rl2glmVec3(mainCamera->target), rl2glmVec3(mainCamera->up));
    ::SetMatrixModelview(*reinterpret_cast<const ::Matrix *>(&view));

    if(skybox && gfxState->renderSkybox)
    {
        DrawModel(skybox->rlSkyboxModel, (Vector3){0, 0, 0}, 1.0f, WHITE);
    }

    DrawPlane({0.0f, -4.0f, 0.0f}, {200.0f, 200.0f}, GREEN);
    // DrawGrid(10, 1.0f);

    entt_state->ecs.view<boyd::comp::Transform, boyd::comp::Mesh>()
        .each([state](auto entity, auto &transform, boyd::comp::Mesh &mesh) {
            memcpy(&mesh.model.transform, &transform.matrix, sizeof(::Matrix));
            DrawModel(mesh.model, (Vector3){0, 6.0f, 0}, 1.0f, WHITE);
        });

    ::EndMode3D();
    ::EndDrawing();
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
