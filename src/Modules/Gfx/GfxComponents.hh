#pragma once

#include <GL/gl.h>
#include <unordered_map>

namespace boyd
{

/// A mesh that is loaded on the GPU.
struct GLMesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
};

/// A material that is loaded on the GPU.
struct GLMaterial
{
    GLuint program;
    GLuint texture;
};

} // namespace boyd
