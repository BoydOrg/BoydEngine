#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/AudioClip.hh"
#include "../Components/AudioSource.hh"
#include "../Components/Camera.hh"
#include "../Components/ComponentLoadRequest.hh"
#include "../Components/Gltf.hh"
#include "../Components/LuaBehaviour.hh"
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

}