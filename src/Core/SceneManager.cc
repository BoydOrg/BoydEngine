#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Components/Camera.hh"
#include "../Components/Mesh.hh"
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

    /// Just instantiate random stuff
    for(int i = 0; i < 2; i++)
    {
        state->entities.push_back(registry.create());
        auto entity = state->entities.back();
        if(i == 0)
        {
            registry.assign<boyd::comp::Camera>(entity, Vector3{10.0f, 10.0f, 10.0f}, 45.0f, CAMERA_PERSPECTIVE);
        }
        else
        {
            registry.assign<boyd::comp::Mesh>(entity, std::string{"assets/guy.iqm"},
                                              std::string{"assets/guytex.png"});
            glm::vec3 position{0.0f, 0.0f, 0.0f};
            glm::mat4 transMatrix{glm::translate(glm::mat4{1.0f}, position)};
            glm::mat4 rotationMatrix{glm::yawPitchRoll(0.0f, DEG2RAD * 90.0f, 0.0f)};
            glm::mat4 modelMatrix = transMatrix * rotationMatrix;

            registry.assign<boyd::comp::Transform>(entity, modelMatrix);
        }
    }
}