#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/AudioSource.hh"
#include "../Components/Camera.hh"
#include "../Components/Mesh.hh"
#include "../Components/Skybox.hh"
#include "../Components/Transform.hh"
#include "GameState.hh"
#include "SceneManager.hh"

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
    registry.assign<boyd::comp::Camera>(mainCam, Vector3{0.0f, 0.0f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    registry.assign<boyd::comp::Skybox>(mainCam, "assets/Textures/dresden_square.hdr");
}
