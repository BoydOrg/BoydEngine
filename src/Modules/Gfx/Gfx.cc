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

    entt_state->ecs.view<boyd::comp::Transform, boyd::comp::Mesh>()
        .each([state](auto entity, auto &transform, auto &mesh) {
            //DrawModelEx(mesh.model, )
            glm::mat4 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform.matrix, scale, rotation, translation, skew, perspective);
        });
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}