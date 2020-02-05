#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Transform.hh"

#include <entt/entt.hpp>

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
    SetCursor(gfxState);
#endif

    ClearBackground(BLACK);

    // Use Raylib's automatic camera management for us
    ::UpdateCamera(mainCamera);
    ::BeginMode3D(*mainCamera);

    DrawPlane({0.0f, 0.0f, 0.0f}, {200.0f, 200.0f}, GREEN);

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