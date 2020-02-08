#pragma once

#include "../../Components/Mesh.hh"
#include "../../Debug/Log.hh"
#include <memory>
#include <utility>
#include <vector>

#include "../Glfw.hh"
#include "GL3Containers.hh"

namespace boyd
{
namespace gl3
{

/// Makes a buffer out of a block of data.
/// Returns 0 on error.
GLuint UploadBuffer(GLenum target, const void *data, size_t dataSize, GLenum usage = GL_STATIC_DRAW);

/// Uploads a mesh from RAM to the GPU.
/// Either generates or updates the given `gpuMesh` to match `mesh`.
/// Returns false on error.
bool UploadMesh(const comp::Mesh &mesh, gl3::Mesh &gpuMesh);

/// Compiles a OpenGL shader.
/// Returns 0 on error.
GLuint CompileShader(GLenum type, std::string source);

/// Links a OpenGL shader program.
/// Returns 0 on error.
GLuint LinkProgram(const std::vector<GLuint> &shaders);

/// A render pass, i.e. a shader program + some settings.
struct RenderPass
{
    /// The shader program to use for this pass.
    GLuint program;
    /// <texture target, texture> pairs for this pass, TEXTURE0..TEXTUREn.
    std::vector<std::pair<GLenum, SharedTexture>> textures;

    /// Creates a new, uninitialized pass.
    RenderPass()
        : program{0}, textures{}
    {
    }

    /// Initializes a pass given its already-loaded `program` and the textures it will use.
    RenderPass(GLuint program, const decltype(textures) textures)
        : program{program}, textures{textures}
    {
    }

    RenderPass(const RenderPass &toCopy) = delete;
    RenderPass &operator=(const RenderPass &toCopy) = delete;

    RenderPass(RenderPass &&toMove)
        : program{toMove.program}, textures{std::move(toMove.textures)}
    {
        toMove.program = 0;
    }
    RenderPass &operator=(RenderPass &&toMove)
    {
        program = toMove.program;
        textures = std::move(toMove.textures);

        toMove.program = 0;

        return *this;
    }

    /// Destroys the program.
    /// NOTE: The textures are passed as SharedTextures, so they are ref-counted!
    ~RenderPass()
    {
        if(program)
        {
            glDeleteProgram(program);
            program = 0;
        }
    }

    /// Binds this pass' program and all of its textures.
    void Begin()
    {
        glUseProgram(program);
        for(GLuint i = 0; i < textures.size(); i++)
        {
            const auto &texPair = textures[i];
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(texPair.first, texPair.second);
        }
    }

    /// Unbinds the program and all of the textures.
    void End()
    {
        for(GLuint i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(textures[i].first, 0);
        }
        glUseProgram(0);
    }
};

} // namespace gl3
} // namespace boyd
