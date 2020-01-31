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

void RegisterMesh(entt::registry &registry, entt::entity entity, boyd::comp::Mesh &mesh)
{
    mesh.model = LoadModel(mesh.modelName.c_str());
    if(mesh.textureName.size())
    {
        mesh.texture = LoadTexture(mesh.textureName.c_str());
        SetMaterialTexture(&mesh.model.materials[0], MAP_DIFFUSE, mesh.texture);
    }
}

void RegisterCamera(entt::registry &registry, entt::entity entity, boyd::comp::Camera &camera)
{
    Camera &raylibCamera = camera.camera;
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
    entt_state->ecs.on_construct<boyd::comp::Mesh>().connect<&RegisterMesh>();
    return new BoydGfxState;
}

BOYD_API void BoydUpdate_Gfx(void *state)
{
    boyd::GameState *entt_state = Boyd_GameState();

    auto *gfxState = GetState(state);
    ::Camera *camera;

    entt_state->ecs.view<boyd::comp::Camera>().each([camera](auto entity, auto &camera) {
        camera = &camera.camera;
    });

    ::BeginMode3D(*camera);

    entt_state->ecs.view<boyd::comp::Transform, boyd::comp::Mesh>()
        .each([state](auto entity, auto &transform, auto &mesh) {
            DrawModelEx(mesh.model, transform.position,
                        transform.rotationAxis,
                        transform.rotationAngle,
                        ::WHITE);
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