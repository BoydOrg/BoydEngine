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

/// A RAII shared pointer over a OpenGL handle.
struct SharedHandle
{
    /// Deletes the inner resource.
    using Deleter = void (*)(GLuint *);

    std::shared_ptr<GLuint> handle;

protected:
    /// Takes ownership of the given resource.
    explicit SharedHandle(GLuint handle, Deleter deleter)
        : handle{new GLuint(handle), deleter}
    {
    }

public:
    virtual ~SharedHandle() = default;

    SharedHandle(const SharedHandle &toCopy) = default;
    SharedHandle &operator=(const SharedHandle &toCopy) = default;
    SharedHandle(SharedHandle &&toMove) = default;
    SharedHandle &operator=(SharedHandle &&toMove) = default;

    inline operator GLuint() const
    {
        return handle ? *handle : 0;
    }
};

/// A RAII shared pointer over an OpenGL buffer.
class SharedBuffer : public SharedHandle
{
public:
    /// Creates a new, empty buffer.
    explicit SharedBuffer()
        : SharedHandle(0, Deleter)
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
class SharedVertexArray : public SharedHandle
{
public:
    using SharedHandle::SharedHandle;

    /// Creates a new vertex array.
    explicit SharedVertexArray()
        : SharedHandle(0, Deleter)
    {
        glGenVertexArrays(1, handle.get());
        if(!*handle)
        {
            BOYD_LOG(Warn, "Failed to create VAO");
        }
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
class SharedTexture : public SharedHandle
{
public:
    /// Takes ownership of the given texture.
    explicit SharedTexture(GLuint handle)
        : SharedHandle(handle, Deleter)
    {
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

/// A RAII shared pointer over an OpenGL shader program.
class SharedProgram : public SharedHandle
{
public:
    /// Takes ownership of the given shader program.
    explicit SharedProgram(GLuint handle)
        : SharedHandle(handle, Deleter)
    {
    }

private:
    static void Deleter(GLuint *value)
    {
        if(value)
        {
            glDeleteProgram(*value);
            delete value;
        }
    }
};

/// A RAII shared pointer over a mesh that is loaded on the GPU.
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

} // namespace gl3
} // namespace boyd