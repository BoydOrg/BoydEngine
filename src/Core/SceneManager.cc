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

    /// Just instantiate random stuff
    for(int i = 0; i < 2; i++)
    {
        state->entities.push_back(registry.create());
        auto entity = state->entities.back();
        if(i == 0)
        {
            registry.assign<boyd::comp::Camera>(entity, Vector3{10.0f, 10.0f, 10.0f}, 45.0f, CAMERA_PERSPECTIVE);
            registry.assign<boyd::comp::Skybox>(entity, "assets/Textures/dresden_square.hdr");
        }
        else
        {
            //registry.assign<boyd::comp::Mesh>(entity, std::string{"assets/GLTF/SuzanneColor0.glb"},
            //                                  std::string{""});
            glm::vec3 position{10.0f, 10.0f, 10.0f};
            glm::mat4 transMatrix{glm::translate(glm::mat4{1.0f}, position)};
            glm::mat4 rotationMatrix{glm::yawPitchRoll(0.0f, 0.0f, 0.0f)};
            glm::mat4 modelMatrix = transMatrix * rotationMatrix;

            registry.assign<boyd::comp::Transform>(entity, modelMatrix);
            // registry.assign<boyd::comp::AudioClip>(entity, boyd::LoadWav("assets/WAV/xp.wav"));
            boyd::comp::ComponentLoadRequest req{{boyd::comp::ComponentLoadRequest::TypeOf<boyd::comp::AudioClip>(), "assets/WAV/xp.wav"}};
            registry.assign<boyd::comp::ComponentLoadRequest>(entity, std::move(req));
            registry.assign<boyd::comp::AudioSource>(entity, boyd::comp::AudioSource::SoundType::SFX_LOOPABLE);

            //registry.assign<boyd::comp::AudioSource>(entity, "assets/WAV/xp.wav",
            //                                         boyd::comp::AudioSource::SoundType::SFX_LOOPABLE);
        }
    }
}