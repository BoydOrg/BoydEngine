#pragma once

#include "../../Components/Mesh.hh"
#include "GfxComponents.hh"
#include <vector>

namespace boyd
{
namespace gl3
{

/// Uploads a mesh from RAM to the GPU.
/// Either generates or updates the given `gpuMesh` to match `mesh`.
/// Returns false on error.
bool UploadMesh(const comp::Mesh &mesh, GLMesh &gpuMesh);

/// Compiles a OpenGL shader.
/// Returns 0 on error.
GLuint CompileShader(GLenum type, std::string source);

/// Links a OpenGL shader program.
/// Returns 0 on error.
GLuint LinkProgram(const std::vector<GLuint> &shaders);

} // namespace gl3
} // namespace boyd
