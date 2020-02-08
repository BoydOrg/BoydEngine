#pragma once

#include "../../Components/Mesh.hh"
#include "../../Debug/Log.hh"
#include <memory>
#include <utility>
#include <vector>

#include "GL3Containers.hh"
#include "Glfw.hh"

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

/// A copy of the standard material (& all of its uniforms + textures).
class StandardMaterial
{
public:
    static constexpr const char *VS_PATH = "assets/Shaders/Standard.vs";
    static constexpr const char *FS_PATH = "assets/Shaders/Standard.fs";

    /// Per-pass uniform data
    struct UPerPass
    {
        GLint diffuseMap; ///< sampler2D u_DiffuseMap;
    };
    /// Per-instance uniform data
    struct UPerInstance
    {
        glm::mat4 modelViewProjection; ///< mat4 u_ModelViewProjection;
    };

private:
    gl3::Material material;         ///< The inner material.
    GLuint nullTexture;             ///< A white 1px*1px texture.
    gl3::SharedBuffer uPerPass;     ///< `UPerPass` in a buffer.
    gl3::SharedBuffer uPerInstance; ///< `UPerInstance` in a buffer.

public:
    /// Loads a copy of the standard material.
    /// Loads & links its shaders and generates a "null" (1px*1px white) texture.
    explicit StandardMaterial();

    StandardMaterial(const StandardMaterial &toCopy) = delete;
    StandardMaterial &operator=(const StandardMaterial &toCopy) = delete;
    StandardMaterial(StandardMaterial &&toMove) = default;
    StandardMaterial &operator=(StandardMaterial &&toMove) = default;

    ~StandardMaterial();

    /// Begins a pass with the standard material (& its default per-pass uniforms).
    inline void BeginPass()
    {
        material.BeginPass(uPerPass);
    }

    /// Sets per-instance uniforms to the given values.
    inline void Instance(const UPerInstance *perInstanceData)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, uPerInstance);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UPerInstance), perInstanceData, GL_STREAM_DRAW);
        material.Instance(uPerInstance);
    }

    /// Ends a pass with the standard material.
    inline void EndPass()
    {
        material.EndPass();
    }
};

} // namespace gl3
} // namespace boyd
