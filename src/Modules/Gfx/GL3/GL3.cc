#include "GL3.hh"

#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

#define BOYD_CHECK(cond, ...)                           \
    if(!(cond))                                         \
    {                                                   \
        BOYD_LOG(Error, "!(" #cond "): ", __VA_ARGS__); \
        return false;                                   \
    }

#define BOYD_OFFSETOF(type, field) \
    reinterpret_cast<void *>(offsetof(type, field))

namespace boyd
{
namespace gl3
{

/// Makes a buffer out of a block of data.
/// Returns 0 on error.
GLuint UploadBuffer(GLenum target, const void *data, size_t dataSize, GLenum usage)
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    BOYD_CHECK(buffer != 0, "Failed to create buffer (target={})", target)

    glBindBuffer(target, buffer);
    glBufferData(target, dataSize, data, usage);
    glBindBuffer(target, 0);

    return buffer;
}

/// Map Mesh::Usage/Texture::Usage to OpenGL usage flags.
static constexpr const GLenum GL_USAGE_MAP[] = {
    GL_STATIC_DRAW,  // Static
    GL_DYNAMIC_DRAW, // Dynamic
    GL_STREAM_DRAW,  // Streaming
};

bool UploadMesh(const comp::Mesh &mesh, gl3::SharedMesh &gpuMesh)
{
    if(gpuMesh.vao == 0)
    {
        gpuMesh.vao = SharedVertexArray();
        BOYD_CHECK(gpuMesh.vao != 0, "Failed to create VAO")
    }
    glBindVertexArray(gpuMesh.vao);

    if(gpuMesh.ibo == 0)
    {
        gpuMesh.ibo = SharedBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                   mesh.data->indices.size() * sizeof(comp::Mesh::Index),
                                   mesh.data->indices.data(),
                                   GL_USAGE_MAP[mesh.data->usage]);
        BOYD_CHECK(gpuMesh.ibo != 0, "Failed to create IBO")
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.ibo);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     mesh.data->indices.size() * sizeof(comp::Mesh::Index),
                     mesh.data->indices.data(),
                     GL_USAGE_MAP[mesh.data->usage]);
    }

    if(gpuMesh.vbo == 0)
    {
        gpuMesh.vbo = SharedBuffer(GL_ARRAY_BUFFER,
                                   mesh.data->vertices.size() * sizeof(comp::Mesh::Vertex),
                                   mesh.data->vertices.data(),
                                   GL_USAGE_MAP[mesh.data->usage]);
        BOYD_CHECK(gpuMesh.vbo != 0, "Failed to create VBO")
        glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.vbo);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.ibo);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.data->vertices.size() * sizeof(comp::Mesh::Vertex),
                     mesh.data->vertices.data(),
                     GL_USAGE_MAP[mesh.data->usage]);
    }

    // Vertex attrib pointers - see the layout of `comp::Mesh::Vertex`!

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, true, //< NOTE: normalized
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, tint));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, texCoord));

    glBindVertexArray(0);
    return true;
}

/// Map Texture::Format to OpenGL <internalFormat, format, input data type>.
struct ImageFormat
{
    GLenum internalFormat;
    GLenum format;
    GLenum dtype;
};
static constexpr const ImageFormat GL_IMAGEFORMAT_MAP[] = {
    // 8-bit int
    {GL_R8, GL_RED, GL_UNSIGNED_BYTE},
    {GL_RG8, GL_RG, GL_UNSIGNED_BYTE},
    {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
    {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
    // 16-bit float
    {GL_R16F, GL_RED, GL_FLOAT},
    {GL_RG16F, GL_RG, GL_FLOAT},
    {GL_RGB16F, GL_RGB, GL_FLOAT},
    {GL_RGBA16F, GL_RGBA, GL_FLOAT},
};

/// Map Texture::Filter to OpenGL filtering modes.
static constexpr const GLenum GL_IMAGEFILTER_MAP[] = {
    GL_NEAREST,              // Nearest
    GL_LINEAR,               // Bilinear
    GL_LINEAR_MIPMAP_LINEAR, // Trilinear
    GL_LINEAR_MIPMAP_LINEAR, // Anisotropic
};

bool UploadTexture(const comp::Texture &texture, gl3::SharedTexture &gpuTexture)
{
    if(gpuTexture == 0)
    {
        gpuTexture = gl3::SharedTexture{0};
        glGenTextures(1, gpuTexture.handle.get());
        BOYD_CHECK(gpuTexture != 0, "Failed to create texture")
    }

    const auto &imgFormat = GL_IMAGEFORMAT_MAP[texture.data->format];

    glBindTexture(GL_TEXTURE_2D, gpuTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 imgFormat.internalFormat,
                 texture.data->width, texture.data->height, 0,
                 imgFormat.format,
                 imgFormat.dtype,
                 texture.data->pixels.data());

    glTexParameteri(gpuTexture, GL_TEXTURE_MIN_FILTER, GL_IMAGEFILTER_MAP[texture.data->minfilter]);
    glTexParameteri(gpuTexture, GL_TEXTURE_MAG_FILTER, GL_IMAGEFILTER_MAP[texture.data->magFilter]);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

GLuint CompileShader(GLenum type, std::string source)
{
    GLuint shader = glCreateShader(type);
    BOYD_CHECK(shader != 0, "Failed to create shader");

    const char *srcStr = source.data();
    glShaderSource(shader, 1, &srcStr, nullptr);
    glCompileShader(shader);

    GLint ok = false;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<GLchar> log(logLen, '\0');
        glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
        BOYD_LOG(Warn, "Failed to compile shader:\n{}", log.data());

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint LinkProgram(const std::vector<GLuint> &shaders)
{
    GLuint program = glCreateProgram();
    BOYD_CHECK(program != 0, "Failed to create shader program");

    for(GLuint shader : shaders)
    {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);
    for(GLuint shader : shaders)
    {
        glDetachShader(program, shader);
    }

    GLint ok = false;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if(!ok)
    {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<GLchar> log(logLen, '\0');
        glGetProgramInfoLog(program, log.size(), nullptr, log.data());
        BOYD_LOG(Warn, "Failed to link shader program:\n{}", log.data());

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

} // namespace gl3
} // namespace boyd
