#include "GL3.hh"

#include "../../Debug/Log.hh"

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

bool UploadMesh(const comp::Mesh &mesh, GLMesh &gpuMesh)
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

} // namespace gl3
} // namespace boyd
