#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../Components/Mesh.hh"
#include "../../Components/Texture.hh"
#include "../../Debug/Log.hh"

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
/// Either generates or updates the given `gpuMesh` to match `mesh` or returns false on error.
bool UploadMesh(const comp::Mesh &mesh, gl3::SharedMesh &gpuMesh);

/// Uploads a texture from RAM to the GPU.
/// Either generates or updates the given `gpuTexture` to match `texture` or returns false on error.
bool UploadTexture(const comp::Texture &texture, gl3::SharedTexture &gpuTexture);

/// Compiles a OpenGL shader.
/// Returns 0 on error.
GLuint CompileShader(GLenum type, std::string source);

/// Links a OpenGL shader program.
/// Returns 0 on error.
GLuint LinkProgram(const std::vector<GLuint> &shaders);

} // namespace gl3
} // namespace boyd
