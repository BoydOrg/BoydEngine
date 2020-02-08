#pragma once

#include "../../Components/Mesh.hh"
#include "Glfw.hh"
#include <vector>

namespace boyd
{
namespace gl3
{

/// A mesh that is loaded on the GPU.
struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    /// Creates a new, uninitialized mesh.
    Mesh()
        : vao{0}, vbo{0}, ibo{0}
    {
    }

    Mesh(const Mesh &toCopy) = delete;
    Mesh &operator=(const Mesh &toCopy) = delete;

    Mesh(Mesh &&toMove)
        : vao{toMove.vao}, vbo{toMove.vbo}, ibo{toMove.ibo}
    {
        toMove.vao = 0;
        toMove.vbo = 0;
        toMove.ibo = 0;
    }
    Mesh &operator=(Mesh &&toMove)
    {
        vao = toMove.vao;
        vbo = toMove.vbo;
        ibo = toMove.ibo;

        toMove.vao = 0;
        toMove.vbo = 0;
        toMove.ibo = 0;

        return *this;
    }

    /// Deletes all of the mesh's VAO/VBO/IBO, if set.
    ~Mesh()
    {
        if(vao)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        if(vbo)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if(ibo)
        {
            glDeleteBuffers(1, &ibo);
            ibo = 0;
        }
    }
};

/// A material that is loaded on the GPU.
struct Material
{
    GLuint program;
    GLuint texture;

    // TODO!
};

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

} // namespace gl3
} // namespace boyd
