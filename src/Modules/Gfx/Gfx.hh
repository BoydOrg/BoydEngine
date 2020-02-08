#pragma once

#include <entt/entt.hpp>
#include <unordered_map>

#include "GL3.hh"
#include "Glfw.hh"

#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

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

/// Called at every gfx reload
void InitInput(BoydGfxState *state);

/// Update the input axes
void UpdateInput(BoydGfxState *state);

} // namespace boyd
