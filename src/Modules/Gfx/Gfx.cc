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

bool BoydGfxState::InitContext()
{
    BOYD_LOG(Debug, "Initializing GLFW");
    if(!glfwInit())
    {
        const char *error;
        int errorCode = glfwGetError(&error);
        BOYD_LOG(Error, "Failed to init GLFW: [{}] {}", errorCode, error);
        return false;
    }

    BOYD_LOG(Debug, "Creating GLFW window");

    glfwWindowHint(GLFW_RESIZABLE, true);
    // Request OpenGL ES 3
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(800, 600, "BoydEngine", nullptr, nullptr);
    if(!window)
    {
        const char *error;
        int errorCode = glfwGetError(&error);
        BOYD_LOG(Error, "Failed to create GLFW window: [{}] {}", errorCode, error);
        return false;
    }
    glfwMakeContextCurrent(window);

    BOYD_LOG(Debug, "OpenGL: {} ({})", glGetString(GL_VERSION), glGetString(GL_VENDOR));
    if(!flextInit(window))
    {
        BOYD_LOG(Error, "flextGL failed!");
        return false;
    }

    return true;
}

gl3::SharedTexture BoydGfxState::MapGpuTexture(comp::Texture *texture)
{
    auto gpuTextureIt = textureMap.find(texture->data.get());
    if(gpuTextureIt != textureMap.end())
    {
        return gpuTextureIt->second;
    }
    else
    {
        gl3::SharedTexture gpuTextureInstance{0};
        bool uploadOk = gl3::UploadTexture(*texture, gpuTextureInstance);
        if(!uploadOk)
        {
            BOYD_LOG(Warn, "Failed to upload texture to GPU");
            return gl3::SharedTexture{0};
        }
        return textureMap.emplace(texture->data.get(), std::move(gpuTextureInstance)).first->second;
    }
}

gl3::SharedMesh BoydGfxState::MapGpuMesh(comp::Mesh *mesh)
{
    auto gpuMeshIt = meshMap.find(mesh->data.get());
    if(gpuMeshIt != meshMap.end())
    {
        return gpuMeshIt->second;
    }
    else
    {
        gl3::SharedMesh gpuMeshInstance;
        bool uploadOk = gl3::UploadMesh(*mesh, gpuMeshInstance);
        if(!uploadOk)
        {
            BOYD_LOG(Warn, "Failed to upload mesh to GPU");
            return {};
        }
        return (meshMap[mesh->data.get()] = std::move(gpuMeshInstance));
    }
}

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

    glfwMakeContextCurrent(window);
    glViewport(0, 0, screenW, screenH);

    // -----------------------------------------------------------------------------------------------------------------
    // Forward pass: render all meshes in the ECS with forward lighting
    // -----------------------------------------------------------------------------------------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    auto &stage = pipeline->stages[gl3::Pipeline::Forward];

    stage.pass.Begin();

    gameState->ecs.view<comp::Transform, comp::Mesh, comp::Material>()
        .each([&](auto entity, const auto &transform, auto &mesh, auto &material) {
            glm::mat4 mvpMtx = viewProjectionMtx * transform.matrix;

            const auto gpuMesh = MapGpuMesh(&mesh);
            // TODO: const auto gpuTexture = GpuTexture(material.param.texture...);

            glBindVertexArray(gpuMesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, nullptr);
        });

    glBindVertexArray(0);
    stage.pass.End();
    glDisable(GL_DEPTH_TEST);

    // -------------------------------------------------------------------------

    glfwSwapBuffers(window);

    // Poll input at end of frame to minimize delay between the Gfx system running (that should be the last one in the sequence)
    // and the next frame
    UpdateInput(this);

    if(glfwWindowShouldClose(window))
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
