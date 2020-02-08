#include "GL3.hh"

#include "../../Debug/Log.hh"
#include "Glfw.hh"

#define BOYD_CHECK(cond, ...)                           \
    if(!(cond))                                         \
    {                                                   \
        BOYD_LOG(Error, "!(" #cond "): ", __VA_ARGS__); \
        return false;                                   \
    }

#define BOYD_OFFSETOF(type, field) \
    reinterpret_cast<void *>(offsetof(type, field))

/// Map Mesh::Usage to OpenGL usage flags.
static constexpr const GLenum GL_USAGE_MAP[] = {
    GL_STATIC_DRAW,  // Static
    GL_DYNAMIC_DRAW, // Dynamic
    GL_STREAM_DRAW,  // Streaming
};

namespace boyd
{
namespace gl3
{

bool UploadMesh(const comp::Mesh &mesh, gl3::Mesh &gpuMesh)
{
    if(gpuMesh.vao == 0)
    {
        glGenVertexArrays(1, &gpuMesh.vao);
        BOYD_CHECK(gpuMesh.vao != 0, "Failed to create VAO")
    }
    if(gpuMesh.vbo == 0)
    {
        glGenBuffers(1, &gpuMesh.vbo);
        BOYD_CHECK(gpuMesh.vbo != 0, "Failed to create VBO")
    }
    if(gpuMesh.ibo == 0)
    {
        glGenBuffers(1, &gpuMesh.ibo);
        BOYD_CHECK(gpuMesh.ibo != 0, "Failed to create IBO")
    }

    glBindVertexArray(gpuMesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.data->vertices.size() * sizeof(comp::Mesh::Vertex), mesh.data->vertices.data(),
                 GL_USAGE_MAP[mesh.data->usage]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.data->indices.size() * sizeof(comp::Mesh::Index), mesh.data->indices.data(),
                 GL_USAGE_MAP[mesh.data->usage]);

    // Vertex attrib pointers - see the layout of `comp::Mesh::Vertex`!
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, true, //< NOTE: normalized
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, tintEmission));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, false,
                          sizeof(comp::Mesh::Vertex), BOYD_OFFSETOF(comp::Mesh::Vertex, texCoord));

    glBindVertexArray(0);
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
