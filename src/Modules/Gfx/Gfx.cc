#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

#include "../../Components/Camera.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"
#include "GL3.hh"
#include "Glfw.hh"

#include <entt/entt.hpp>
#include <unordered_map>

namespace boyd
{

static constexpr const char *STANDARD_VS_PATH = "assets/Shaders/Standard.vs";
static constexpr const char *STANDARD_FS_PATH = "assets/Shaders/Standard.fs";

struct BoydGfxState
{
    GLFWwindow *window;

    /// Maps all mesh data to its respective OpenGL mesh info.
    /// This is so implicit sharing for mesh data works seamlessly - if the mesh data is identical it should go on the GPU just once!
    std::unordered_map<comp::Mesh::Data *, gl3::Mesh> meshMap;

    struct
    {
        GLuint handle;
        GLuint nullTextureHandle;   ///< A handle to a 1x1px white (aka "null") texture.
        GLint uModelViewProjection; ///< `u_ModelViewProjection`
        GLint uTexture;             ///< `u_Texture`

    } standardSP; ///< Standard shader program.

    BoydGfxState()
    {
        if(!InitContext())
        {
            return;
        }
        if(!InitStandardShader())
        {
            return;
        }
    }

    ~BoydGfxState()
    {
        // Important: destroy all OpenGL data before terminating GLFW!
        meshMap.clear();
        glDeleteProgram(standardSP.handle);
        standardSP.handle = 0;
        glDeleteTextures(1, &standardSP.nullTextureHandle);
        standardSP.nullTextureHandle = 0;

        // Deinit GLFW
        if(window)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    /// Remove all meshes from the GPU that have are unused.
    void CollectGarbage()
    {
        // FIXME IMPLEMENT: To do this, need a wait to reference-count the meshes in `meshMap`
        //                  and kill them only when the reference count is one (i.e., just the pointer in the map)
        //                  - need to store the `shared_ptr` directly as key?
    }

private:
    /// Initialize GLFW and flextGL.
    bool InitContext()
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

    /// Load the standard shader program.
    bool InitStandardShader()
    {
        BOYD_LOG(Debug, "Loading the standard shader program (VS={}, FS={})", STANDARD_VS_PATH, STANDARD_FS_PATH);

        std::string shaderSource;

        if(!Slurp(STANDARD_VS_PATH, shaderSource))
        {
            BOYD_LOG(Error, "Failed to load the standard VS", STANDARD_VS_PATH);
            return false;
        }
        GLuint vs = gl3::CompileShader(GL_VERTEX_SHADER, shaderSource);
        if(vs == 0)
        {
            BOYD_LOG(Error, "Failed to compile the standard VS");
            return false;
        }

        if(!Slurp(STANDARD_FS_PATH, shaderSource))
        {
            BOYD_LOG(Error, "Failed to load the standard FS from {}!", STANDARD_FS_PATH);
            return false;
        }
        GLuint fs = gl3::CompileShader(GL_FRAGMENT_SHADER, shaderSource);
        if(fs == 0)
        {
            BOYD_LOG(Error, "Failed to compile the standard FS");
            glDeleteShader(vs);
            return false;
        }

        standardSP.handle = gl3::LinkProgram({vs, fs});
        if(standardSP.handle == 0)
        {
            BOYD_LOG(Error, "Failed to link the standard shader program");
            glDeleteShader(vs);
            glDeleteShader(fs);
            return false;
        }

        standardSP.uModelViewProjection = glGetUniformLocation(standardSP.handle, "u_ModelViewProjection");
        standardSP.uTexture = glGetUniformLocation(standardSP.handle, "u_Texture");

        // Generate the "null" texture
        glGenTextures(1, &standardSP.nullTextureHandle);
        glBindTexture(GL_TEXTURE_2D, standardSP.nullTextureHandle);

        static constexpr const uint8_t WHITE[3] = {255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGB8, 1, 1, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, WHITE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
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
    glfwPollEvents();

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
