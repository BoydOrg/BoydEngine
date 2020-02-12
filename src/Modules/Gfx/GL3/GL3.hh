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

/// Map Mesh::Usage/Texture::Usage to OpenGL usage flags.
extern const GLenum GL_USAGE_MAP[];

/// Map Texture::Type to OpenGL texture type.
extern const GLenum GL_TEXTURETYPE_MAP[];

/// Map Texture::Format to OpenGL <internalFormat, format, input data type>.
struct ImageFormat
{
    GLenum internalFormat;
    GLenum format;
    GLenum dtype;
    size_t dtypeSize;
};
extern const ImageFormat GL_IMAGEFORMAT_MAP[];

/// Map Texture::Filter to OpenGL filtering modes.
extern const GLenum GL_IMAGEFILTER_MAP[];

/// Makes a buffer out of a block of data.
/// Returns 0 on error.
GLuint UploadBuffer(GLenum target, const void *data, size_t dataSize, GLenum usage = GL_STATIC_DRAW);

/// Uploads a mesh from RAM to the GPU.
/// Creates a VBO, IBO and VAO for `gpuMesh` if required - otherwise just changes the contained data.
/// Either generates or updates the given `gpuMesh` to match `mesh` or returns false on error.
bool UploadMesh(const comp::Mesh &mesh, gl3::SharedMesh &gpuMesh);

/// Uploads a texture from RAM to the GPU.
/// Creates a texture for `gpuTexture` if required - otherwise just changes the contained data.
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
