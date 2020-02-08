#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

#include "GL3/GL3.hh"
#include "GL3/GL3Pipeline.hh"
#include "Glfw.hh"

#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

namespace boyd
{

struct BoydGfxState
{
    GLFWwindow *window;

    /// Maps all mesh data to its respective OpenGL mesh info.
    /// This is so implicit sharing for mesh data works seamlessly - if the mesh data is identical it should go on the GPU just once!
    std::unordered_map<comp::Mesh::Data *, gl3::Mesh> meshMap;

    /// The whole rendering pipeline.
    std::unique_ptr<gl3::Pipeline> pipeline;

    BoydGfxState()
    {
        if(!InitContext())
        {
            return;
        }
        pipeline = std::make_unique<gl3::Pipeline>();
    }

    ~BoydGfxState()
    {
        // Important: destroy all OpenGL data before terminating GLFW!
        meshMap.clear();
        pipeline.reset();

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

    /// Does all necessary steps to poll events and render a frame.
    void Update();

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
};

/// Called at every gfx reload
void InitInput(BoydGfxState *state);

/// Update the input axes
void UpdateInput(BoydGfxState *state);

} // namespace boyd
