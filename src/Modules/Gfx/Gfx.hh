#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

#include "GL3/GL3.hh"
#include "GL3/GL3Pipeline.hh"
#include "Glfw.hh"

#include "../../Components/Material.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Texture.hh"
#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

namespace boyd
{

struct BoydGfxState
{
    GLFWwindow *window;

    /// The whole rendering pipeline.
    std::unique_ptr<gl3::Pipeline> pipeline;

    /// Maps all mesh data to its respective OpenGL mesh info.
    /// (This is so implicit sharing for mesh data works seamlessly: 1 comp::Mesh on RAM -> 1 OpenGL mesh on VRAM)
    std::unordered_map<comp::Mesh::Data *, gl3::SharedMesh> meshMap;

    /// Maps all textures to their respective OpenGL counterpart.
    /// (This is so implicit sharing for texture data works seamlessly: 1 comp::Texture on RAM -> 1 OpenGL texture on VRAM)
    std::unordered_map<comp::Texture::Data *, gl3::SharedTexture> textureMap;

public:
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
    bool InitContext();

    /// Gets the GPU texture from `textureMap` that is mapped to the given texture in RAM.
    /// If there isn't any uploads `texture` to VRAM, then adds the pair to `textureMap` and returns the freshly-uploaded GPU texture.
    gl3::SharedTexture MapGpuTexture(const comp::Texture *texture);

    /// Gets the GPU mesh from `meshMap` that is mapped to the given mesh in RAM.
    /// If there isn't any uploads `mesh` to VRAM, then adds the pair to `meshMap` and returns the freshly-uploaded GPU mesh.
    gl3::SharedMesh MapGpuMesh(const comp::Mesh *mesh);

    /// Applies all of a material's parameters to `program`.
    /// Loads and bits textures (via `MapGpuTexture()`) as necessary.
    /// Returns the number of textures bound.
    ///
    /// WARNING: Assumes tha `pass.program` is bound!
    unsigned ApplyMaterialParams(const comp::Material &material, gl3::SharedProgram &program);
};

/// Called at every gfx reload
void InitInput(BoydGfxState *state);

/// Update the input axes
void UpdateInput(BoydGfxState *state);

} // namespace boyd
