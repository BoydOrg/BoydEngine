#include "Gfx.hh"

#include <entt/entt.hpp>

#include "../../Components/Camera.hh"
#include "../../Components/Material.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"

#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
using namespace boyd;

#include "GL3/GL3.hh"
#include "Glfw.hh"

namespace boyd
{

void BoydGfxState::Update()
{
    auto *gameState = Boyd_GameState();

    if(!window)
    {
        // Can't to much without a window
        return;
    }
    int screenW, screenH;
    glfwGetFramebufferSize(window, &screenW, &screenH);
    glm::vec2 screenSize{screenW, screenH};

    // TODO: Need a way to actually pick what camera to use - find the camera tagged "MainCamera"?
    auto cameraView = gameState->ecs.view<comp::Camera, comp::ActiveCamera>();
    if(cameraView.empty())
    {
        // Hard to render anything without a camera...
        return;
    }
    auto cameraEntity = *cameraView.begin();
    const auto &camera = gameState->ecs.get<comp::Camera>(cameraEntity);

    glm::mat4 projMtx;
    switch(camera.mode)
    {
    case comp::Camera::Persp:
        projMtx = glm::perspectiveFov(camera.fov, screenSize.x, screenSize.y, camera.zNear, camera.zFar);
        break;
    default: // comp::Camera::Ortho
        if(glm::isinf(camera.zNear) || glm::isinf(camera.zFar))
        {
            projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top);
        }
        else
        {
            projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top, camera.zNear, camera.zFar);
        }
        break;
    }

    glm::mat4 viewMtx = glm::identity<glm::mat4>();
    if(gameState->ecs.has<comp::Transform>(cameraEntity))
    {
        viewMtx = gameState->ecs.get<comp::Transform>(cameraEntity).matrix;
        viewMtx = glm::inverse(viewMtx); // Inverse, because this is a camera view matrix!
    }

    glm::mat4 viewProjectionMtx = projMtx * viewMtx;

    // -------------------------------------------------------------------------

    auto &stage = pipeline->stages[gl3::Pipeline::Forward];

    glfwMakeContextCurrent(window);
    glViewport(0, 0, screenW, screenH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    stage.pass.Begin();

    gameState->ecs.view<comp::Transform, comp::Mesh, comp::Material>()
        .each([&](auto entity, const auto &transform, const auto &mesh, const auto &material) {
            // Find if we have the OpenGL state for this mesh data;
            // if not, upload it to GPU.
            auto gpuMeshIt = gfxState->meshMap.find(mesh.data.get());
            gl3::Mesh *gpuMesh;
            if(gpuMeshIt != gfxState->meshMap.end())
            {
                gpuMesh = &gpuMeshIt->second;
            }
            else
            {
                gl3::Mesh gpuMeshInstance;
                bool uploadOk = gl3::UploadMesh(mesh, gpuMeshInstance);
                if(!uploadOk)
                {
                    BOYD_LOG(Warn, "Failed to upload mesh to GPU");
                    return;
                }
                gpuMesh = &(gfxState->meshMap[mesh.data.get()] = std::move(gpuMeshInstance));
            }

            glm::mat4 mvpMtx = viewProjectionMtx * transform.matrix;
            memcpy(&uPerInstance.modelViewProjection, &mvpMtx, sizeof(glm::mat4));

            gfxState->stdMaterial.Instance(&uPerInstance);
            glBindVertexArray(gpuMesh->vao);
            glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, nullptr);
        });

    glBindVertexArray(0);
    stage.pass.End();
    glDisable(GL_DEPTH_TEST);

    // -------------------------------------------------------------------------

    glfwSwapBuffers(gfxState->window);

    // Poll input at end of frame to minimize delay between the Gfx system running (that should be the last one in the sequence)
    // and the next frame
    UpdateInput(gfxState);

    if(glfwWindowShouldClose(gfxState->window))
    {
        gameState->running = false;
    }
}

} // namespace boyd

inline BoydGfxState *GetState(void *state)
{
    return reinterpret_cast<boyd::BoydGfxState *>(state);
}

extern "C" {

BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    auto *state = new BoydGfxState;
    InitInput(state);
    return state;
}

BOYD_API void BoydUpdate_Gfx(void *statePtr)
{
    auto *gfxState = GetState(statePtr);
    gfxState->Update();
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
