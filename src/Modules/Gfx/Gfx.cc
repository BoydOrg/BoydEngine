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
};

inline BoydGfxState *GetState(void *state)
{
    return reinterpret_cast<BoydGfxState *>(state);
}

void OnRegisterMesh(entt::registry &registry, entt::entity entity)
{
    auto &mesh = registry.get<boyd::comp::Mesh>(entity);

    mesh.model = LoadModel(mesh.modelName.c_str());
    if(mesh.textureName.size())
    {
        mesh.texture = LoadTexture(mesh.textureName.c_str());
        SetMaterialTexture(&mesh.model.materials[0], MAP_DIFFUSE, mesh.texture);
    }
}

void OnRegisterCamera(entt::registry &registry, entt::entity entity)
{
    auto &camera = registry.get<boyd::comp::Camera>(entity);

    RaylibCamera &raylibCamera = camera.camera;
    // TODO: move this somewhere else
    raylibCamera.position = (Vector3){0.0, 1.0, 0.0f};
    raylibCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
    raylibCamera.target = (Vector3){0.0f, 0.0f, 0.0f};
    raylibCamera.fovy = 45.0f;
    raylibCamera.type = CAMERA_PERSPECTIVE;
}

extern "C" {
BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    boyd::GameState *entt_state = Boyd_GameState();
    entt_state->ecs.on_construct<boyd::comp::Mesh>().template connect<&OnRegisterMesh>();

    // entt_state->ecs.on_construct<boyd::comp::Camera>().connect<&OnRegisterCamera>();
    return new BoydGfxState;
}

BOYD_API void BoydUpdate_Gfx(void *state)
{
    boyd::GameState *entt_state = Boyd_GameState();

    auto *gfxState = GetState(state);
    Camera *mainCamera;

    entt_state->ecs.view<boyd::comp::Camera>().each([&mainCamera](auto entity, auto &camera) {
        mainCamera = &camera.camera;
    });

    ::BeginMode3D(*mainCamera);

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
    boyd::GameState *entt_state = Boyd_GameState();
    entt_state->ecs.on_construct<boyd::comp::Mesh>().disconnect<&OnRegisterMesh>();
    delete GetState(state);
}
}