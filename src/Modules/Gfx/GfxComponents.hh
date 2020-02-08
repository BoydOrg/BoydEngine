#pragma once

#include "Glfw.hh"
#include <unordered_map>

namespace boyd
{

/// A mesh that is loaded on the GPU.
struct GLMesh
{
    GLuint vao{0};
    GLuint vbo{0};
    GLuint ibo{0};
};

/// A material that is loaded on the GPU.
struct GLMaterial
{
    GLuint program{0};
    GLuint texture{0};
};

} // namespace boyd
