#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"
#include "GL3.hh"
#include "GfxComponents.hh"

#include <entt/entt.hpp>
#include <unordered_map>

namespace boyd
{

/// TODO: add state transfer
struct BoydGfxState
{
    GLFWwindow *window;

    /// Maps all mesh data to its respective OpenGL mesh info.
    /// This is so implicit sharing for mesh data works seamlessly - if the mesh data is identical it should go on the GPU just once!
    std::unordered_map<comp::Mesh::Data *, GLMesh> meshMap;

    BoydGfxState()
    {
        BOYD_LOG(Debug, "Initializing GLFW");
        if(!glfwInit())
        {
            const char *error;
            int errorCode = glfwGetError(&error);
            BOYD_LOG(Error, "Failed to init GLFW: [{}] {}", errorCode, error);
            return;
        }

        BOYD_LOG(Debug, "Creating GLFW window");

        glfwWindowHint(GLFW_RESIZABLE, true);
        // Request OpenGL ES 3
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        window = glfwCreateWindow(800, 600, "BoydEngine", nullptr, nullptr);
        if(!window)
        {
            const char *error;
            int errorCode = glfwGetError(&error);
            BOYD_LOG(Error, "Failed to create GLFW window: [{}] {}", errorCode, error);
            return;
        }
        glfwMakeContextCurrent(window);

        BOYD_LOG(Debug, "OpenGL: {} ({})", glGetString(GL_VERSION), glGetString(GL_VENDOR));
    }

    ~BoydGfxState()
    {
        if(window)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

} // namespace boyd

using namespace boyd;

inline BoydGfxState *GetState(void *state)
{
    return reinterpret_cast<boyd::BoydGfxState *>(state);
}

extern "C" {

BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    return new BoydGfxState;
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

    // TODO comp::Skybox *skybox = nullptr;
    // TODO if(gameState->ecs.has<comp::Skybox>(cameraEntity))
    // TODO {
    // TODO     skybox = &gameState->ecs.get<comp::Skybox>(cameraEntity);
    // TODO }

    // -------------------------------------------------------------------------

    glfwMakeContextCurrent(gfxState->window);

    // FIXME: BIND THE SHADER PROGRAM FOR MESHES!
    // FIXME: ONLY RENDER MESHES WITH MESHRENDERERS COMPONENTS IN THEM!

    gameState->ecs.view<comp::Transform, comp::Mesh>()
        .each([gfxState](auto entity, const auto &transform, auto &mesh) {
            // Find if we have the OpenGL state for this mesh data;
            // if not, upload it to GPU.
            auto gpuMeshIt = gfxState->meshMap.find(mesh.data.get());
            GLMesh *gpuMesh;
            if(gpuMeshIt != gfxState->meshMap.end())
            {
                gpuMesh = &gpuMeshIt->second;
            }
            else
            {
                GLMesh gpuMeshInstance;
                bool uploadOk = gl3::UploadMesh(mesh, gpuMeshInstance);
                if(!uploadOk)
                {
                    BOYD_LOG(Warn, "Failed to upload mesh to GPU");
                    return;
                }
                gpuMesh = &(gfxState->meshMap[mesh.data.get()] = std::move(gpuMeshInstance));
            }

            glBindVertexArray(gpuMesh->vao);
            glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, nullptr);
        });

    glUseProgram(0);

    // -------------------------------------------------------------------------

    glfwSwapBuffers(gfxState->window);

    // Poll input at end of frame to minimize delay between the Gfx system running (that should be the last one in the sequence)
    // and the next frame
    glfwPollEvents();
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
