#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"
#include "GL3.hh"
#include "Gfx.hh"
#include "Glfw.hh"

#include <entt/entt.hpp>

using namespace boyd;

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
    GameState *gameState = Boyd_GameState();
    auto *gfxState = GetState(statePtr);
    if(!gfxState->window)
    {
        // Can't to much without a window
        return;
    }

    int screenW, screenH;
    glfwGetFramebufferSize(gfxState->window, &screenW, &screenH);
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

    // TODO comp::Skybox *skybox = nullptr;
    // TODO if(gameState->ecs.has<comp::Skybox>(cameraEntity))
    // TODO {
    // TODO     skybox = &gameState->ecs.get<comp::Skybox>(cameraEntity);
    // TODO }

    // -------------------------------------------------------------------------

    glfwMakeContextCurrent(gfxState->window);
    glViewport(0, 0, screenW, screenH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // TODO: ONLY RENDER MESHES WITH MESHRENDERERS COMPONENTS IN THEM!
    // TODO: HANDLE MATERIAL COMPONENTS!

    glUseProgram(gfxState->standardSP.handle);
    glEnable(GL_DEPTH_TEST);

    gameState->ecs.view<comp::Transform, comp::Mesh>()
        .each([&](auto entity, const auto &transform, auto &mesh) {
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
            glUniformMatrix4fv(gfxState->standardSP.uModelViewProjection, 1, false, &mvpMtx[0][0]);

            // TODO: Handle the real texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gfxState->standardSP.nullTextureHandle);
            glUniform1i(gfxState->standardSP.uTexture, 0); // (TEXTURE0)

            glBindVertexArray(gpuMesh->vao);
            glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, nullptr);
        });

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

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

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
