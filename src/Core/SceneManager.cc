#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/AudioClip.hh"
#include "../Components/AudioSource.hh"
#include "../Components/Camera.hh"
#include "../Components/ComponentLoadRequest.hh"
#include "../Components/Gltf.hh"
#include "../Components/Material.hh"
#include "../Components/Mesh.hh"
#include "../Components/Skybox.hh"
#include "../Components/Transform.hh"

#include "GameState.hh"
#include "SceneManager.hh"
#include <utility>

extern "C" boyd::SceneManagerState *Boyd_SceneManager()
{
    return boyd::SceneManager::Instance();
}

void boyd::SceneManager::LoadScene(const std::filesystem::path &scene)
{
    (void)scene; // suppress compiler warnings
    auto &registry = Boyd_GameState()->ecs;
    auto *state = Boyd_SceneManager();

    // Just instantiate random stuff
    /// The camera is instantiated in scripts/main.lua now :)

    auto testCube = registry.create();
    registry.assign<boyd::comp::Transform>(testCube, glm::translate(glm::identity<glm::mat4>(), glm::vec3{0.0f, 0.0f, -3.0f}));

    boyd::comp::ComponentLoadRequest cubeReq{
        {boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::Gltf>(), "assets/GLTF/TexturedCube.glb"},
    };
    registry.assign<boyd::comp::ComponentLoadRequest>(testCube, std::move(cubeReq));
}
