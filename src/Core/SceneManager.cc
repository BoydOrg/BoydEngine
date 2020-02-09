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
    auto mainCam = registry.create();
    registry.assign<boyd::comp::Transform>(mainCam,
                                           glm::lookAt(glm::vec3{-10.0f, -5.0f, -10.0f}, glm::vec3{0.0f, 5.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}));
    registry.assign<boyd::comp::Camera>(mainCam, boyd::comp::Camera::Perspective(90.0f));
    registry.assign<boyd::comp::ActiveCamera>(mainCam);
    //registry.assign<boyd::comp::Skybox>(mainCam, "assets/Textures/dresden_square.hdr");

    auto object = registry.create();
    registry.assign<boyd::comp::Transform>(object, glm::translate(glm::rotate(glm::identity<glm::mat4>(), glm::pi<float>(), {0.0f, 1.0f, 0.0f}), glm::vec3{0.0f, 5.0f, 0.0f}));
    auto &material = registry.assign<boyd::comp::Material>(object);
    material.parameters["DiffuseMap"] = comp::Texture({
        comp::Texture::RGB8,
        1,
        1,
        {255, 255, 255},
        comp::Texture::Nearest,
        comp::Texture::Nearest,
        comp::Texture::Static,
    });

    auto object2 = registry.create();
    registry.assign<boyd::comp::Transform>(object2, glm::translate(glm::identity<glm::mat4>(), glm::vec3{0.0f, 0.0f, 0.0f}));

    boyd::comp::ComponentLoadRequest suzanneGltf{
        {boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::Gltf>(), "assets/GLTF/SuzanneColor0.glb"},
    };
    boyd::comp::ComponentLoadRequest cubeReq{
        {boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::Gltf>(), "assets/GLTF/SampleCube.gltf.glb"},
    };
    registry.assign<boyd::comp::ComponentLoadRequest>(object, std::move(suzanneGltf));
    registry.assign<boyd::comp::ComponentLoadRequest>(object2, std::move(cubeReq));
}
