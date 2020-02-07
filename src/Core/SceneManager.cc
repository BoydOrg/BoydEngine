#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/AudioClip.hh"
#include "../Components/AudioSource.hh"
#include "../Components/Camera.hh"
#include "../Components/ComponentLoadRequest.hh"
#include "../Components/Mesh.hh"
#include "../Components/Skybox.hh"
#include "../Components/Transform.hh"
#include "GameState.hh"
#include "MusicAssetLoader.hh"
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
    auto mainCam = registry.create();
    registry.assign<boyd::comp::Transform>(mainCam,
                                           glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{10.0f, 10.0f, 10.0f}, glm::vec3{0.0f, 1.0f, 0.0f}));
    registry.assign<boyd::comp::Camera>(mainCam, boyd::comp::Camera::Perspective(90.0f));
    registry.assign<boyd::comp::ActiveCamera>(mainCam);
    //registry.assign<boyd::comp::Skybox>(mainCam, "assets/Textures/dresden_square.hdr");

    auto object = registry.create();
    registry.assign<boyd::comp::Transform>(object, glm::identity<glm::mat4>());
    //registry.assign<boyd::comp::Mesh>(object, "assets/GLTF/SuzanneColor0.glb");
}
