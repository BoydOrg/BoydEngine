#pragma once

#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include <memory>
#include <utility>
#include <vector>

#include "../Glfw.hh"

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

    /// Takes ownership of the given buffer.
    explicit SharedBuffer(GLuint handle)
        : SharedHandle(handle, Deleter)
    {
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

    /// Takes ownership of the given vertex array.
    explicit SharedVertexArray(GLuint handle)
        : SharedHandle(handle, Deleter)
    {
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
    /// Cache of <uniform name, uniform location>, for convenience (& performance).
    std::unordered_map<std::string, GLint> uniforms;

    /// Creates a new, uninitialized (0) program.
    SharedProgram()
        : SharedHandle(0, Deleter)
    {
    }

    /// Takes ownership of the given shader program.
    /// Initializes a ShaderProgram given its already-loaded `program`.
    /// Queries all uniform locations in `program`.
    explicit SharedProgram(GLuint handle)
        : SharedHandle(handle, Deleter)
    {
        if(handle == 0)
        {
            return;
        }

        GLint nUniforms = 0;
        glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &nUniforms);
        //BOYD_LOG(Debug, "Program {} has {} uniforms", handle, nUniforms);

        GLchar uniformName[64];
        GLsizei uniformNameLen;
        GLint uSize;
        GLenum uType;
        for(GLint i = 0; i < nUniforms; i++)
        {
            glGetActiveUniform(handle, GLuint(i), sizeof(uniformName), &uniformNameLen, &uSize, &uType, uniformName);
            std::string nameStr{uniformName, size_t(uniformNameLen)};
            uniforms[nameStr] = i;
            //BOYD_LOG(Debug, "Program {}: uniform {} is {}", handle, i, nameStr);
        }
    }

    /// Potentially faster version of `glUniformLocation`; uses the internal map to do the lookup.
    inline GLint uniformLocation(const std::string &name)
    {
#ifndef BOYD_PLATFORM_EMSCRIPTEN
        auto uniformLocIt = uniforms.find(name);
        if(uniformLocIt == uniforms.end())
        {
            return -1;
        }
        return uniformLocIt->second;
#else
        // FIXME: Seems to be an Emscripten bug; not querying the uniform location explictly causes the uniform not to be set correctly!
        return glGetUniformLocation(*handle, name.c_str());
#endif
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
struct SharedMesh
{
    SharedVertexArray vao;
    SharedBuffer vbo;
    SharedBuffer ibo;

    /// Creates a new, uninitialized mesh.
    SharedMesh()
        : vao{0}, vbo{0}, ibo{0}
    {
    }
    /// Creates a new mesh from the given VAO and buffers (that will be implicitly shared).
    SharedMesh(SharedVertexArray vao, SharedBuffer vbo, SharedBuffer ibo)
        : vao{vao}, vbo{vbo}, ibo{ibo}
    {
    }

    ~SharedMesh() = default;
};

} // namespace gl3
} // namespace boyd
