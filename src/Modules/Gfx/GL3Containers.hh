#pragma once

#include "../../Debug/Log.hh"
#include "Glfw.hh"
#include <memory>
#include <utility>
#include <vector>

namespace boyd
{
namespace gl3
{

/// A RAII shared pointer over an OpenGL buffer.
struct SharedBuffer
{
    std::shared_ptr<GLuint> handle;

    /// Takes ownership of the given buffer.
    explicit SharedBuffer(GLuint handle)
        : handle{new GLuint(handle), &Deleter}
    {
    }
    /// Creates a new, empty buffer.
    explicit SharedBuffer()
        : handle{new GLuint(0), &Deleter}
    {
        glGenBuffers(1, handle.get());
        if(!*handle)
        {
            BOYD_LOG(Warn, "Failed to create buffer");
            return;
        }
    }
    /// Creates a new buffer with the given data.
    SharedBuffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage = GL_STATIC_DRAW)
        : SharedBuffer()
    {
        if(handle && *handle != 0)
        {
            glBindBuffer(target, *handle);
            glBufferData(target, size, data, usage);
            glBindBuffer(target, 0);
        }
    }

    SharedBuffer(const SharedBuffer &toCopy) = default;
    SharedBuffer &operator=(const SharedBuffer &toCopy) = default;
    SharedBuffer(SharedBuffer &&toMove) = default;
    SharedBuffer &operator=(SharedBuffer &&toMove) = default;

    inline operator GLuint() const
    {
        return handle ? *handle : 0;
    }

private:
    static void Deleter(GLuint *value)
    {
        if(value)
        {
            glDeleteBuffers(1, value);
            delete value;
        }
    }
};

/// A RAII shared pointer over an OpenGL VAO.
struct SharedVertexArray
{
    std::shared_ptr<GLuint> handle;

    /// Takes ownership of the given vertex array.
    explicit SharedVertexArray(GLuint handle)
        : handle{new GLuint(handle), &Deleter}
    {
    }
    /// Creates a new vertex array.
    explicit SharedVertexArray()
        : SharedVertexArray(0)
    {
        glGenVertexArrays(1, handle.get());
        if(!*handle)
        {
            BOYD_LOG(Warn, "Failed to create VAO");
            return;
        }
    }

    SharedVertexArray(const SharedVertexArray &toCopy) = default;
    SharedVertexArray &operator=(const SharedVertexArray &toCopy) = default;
    SharedVertexArray(SharedVertexArray &&toMove) = default;
    SharedVertexArray &operator=(SharedVertexArray &&toMove) = default;

    inline operator GLuint() const
    {
        return handle ? *handle : 0;
    }

private:
    static void Deleter(GLuint *value)
    {
        if(value)
        {
            glDeleteVertexArrays(1, value);
            delete value;
        }
    }
};

/// A RAII shared pointer over an OpenGL texture.
struct SharedTexture
{
    std::shared_ptr<GLuint> handle;

    /// Takes ownership of the given texture.
    explicit SharedTexture(GLuint handle)
        : handle{new GLuint(handle), &Deleter}
    {
    }

    SharedTexture(const SharedTexture &toCopy) = default;
    SharedTexture &operator=(const SharedTexture &toCopy) = default;
    SharedTexture(SharedTexture &&toMove) = default;
    SharedTexture &operator=(SharedTexture &&toMove) = default;

    inline operator GLuint() const
    {
        return handle ? *handle : 0;
    }

private:
    static void Deleter(GLuint *value)
    {
        if(value)
        {
            glDeleteTextures(1, value);
            delete value;
        }
    }
};

/// A mesh that is loaded on the GPU.
struct Mesh
{
    SharedVertexArray vao;
    SharedBuffer vbo;
    SharedBuffer ibo;

    /// Creates a new, uninitialized mesh.
    Mesh()
        : vao{0}, vbo{0}, ibo{0}
    {
    }
    /// Creates a new mesh from the given VAO and buffers (that will be implicitly shared).
    Mesh(SharedVertexArray vao, SharedBuffer vbo, SharedBuffer ibo)
        : vao{vao}, vbo{vbo}, ibo{ibo}
    {
    }

    Mesh(const Mesh &tocopy) = delete;
    Mesh &operator=(const Mesh &tocopy) = delete;
    Mesh(Mesh &&tomove) = default;
    Mesh &operator=(Mesh &&tomove) = default;

    ~Mesh() = default;
};

/// A material that is loaded on the GPU.
struct Material
{
    /// Name of the per-pass block of uniforms.
    static constexpr const char *U_PERPASS_NAME = "Pass";
    /// Name of the per-instance block of uniforms.
    static constexpr const char *U_PERINSTANCE_NAME = "Instance";
    /// Uniform block buffer bindpoints.
    enum BlockBindings : GLuint
    {
        PERPASS = 1,
        PERINSTANCE = 2,
    };

    /// Handle to the shader program.
    GLuint program;
    /// Contains <texture target, texture handle> pairs for the textures to bind, TEXTURE0..TEXTUREn.
    std::vector<std::pair<GLenum, GLuint>> textures;

    /// Creates a new, uninitialized material.
    Material()
        : program{0}, textures{}
    {
    }

    /// Initializes a material given its already-loaded `program`, how many textures it will use,
    /// and a (optional) handle to the per-pass uniform buffer.
    Material(GLuint program, unsigned nTextures = 0)
        : program{program}, textures{nTextures, {GL_TEXTURE_2D, 0}}
    {
        // Bind uniform blocks to their target locations
        GLuint uPerPass = glGetUniformBlockIndex(program, U_PERPASS_NAME);
        glUniformBlockBinding(program, uPerPass, PERPASS);
        GLuint uPerInstance = glGetUniformBlockIndex(program, U_PERINSTANCE_NAME);
        glUniformBlockBinding(program, uPerInstance, PERINSTANCE);
    }

    Material(const Material &toCopy) = delete;
    Material &operator=(const Material &toCopy) = delete;

    Material(Material &&toMove)
        : program{toMove.program}, textures{std::move(toMove.textures)}
    {
        toMove.program = 0;
    }
    Material &operator=(Material &&toMove)
    {
        program = toMove.program;
        textures = std::move(toMove.textures);

        toMove.program = 0;

        return *this;
    }

    /// Destroys the program (but NOT any uniform buffer or any of the textures!)
    ~Material()
    {
        if(program)
        {
            glDeleteProgram(program);
            program = 0;
        }
    }

    /// Binds a pass of the shader program given the per-pass uniforms buffer.
    void BeginPass(GLuint uPerPassBuffer)
    {
        glUseProgram(program);
        glBindBufferBase(GL_UNIFORM_BUFFER, PERPASS, uPerPassBuffer);
        for(GLuint i = 0; i < textures.size(); i++)
        {
            const auto &texPair = textures[i];
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(texPair.first, texPair.second);
        }
    }

    /// Binds the per-instance uniform buffer into the shader program.
    void Instance(GLuint uPerInstanceBuffer)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, PERINSTANCE, uPerInstanceBuffer);
    }

    /// Unbinds the program and all of the textures.
    void EndPass()
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, PERPASS, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, PERINSTANCE, 0);
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