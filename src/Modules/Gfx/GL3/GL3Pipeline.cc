#include "GL3Pipeline.hh"

#include "../../Core/Utils.hh"
#include "../../Debug/Log.hh"

namespace boyd
{
namespace gl3
{

struct ShaderPaths
{
    const char *vertex;
    const char *fragment;
};
/// Shader filepaths, indexed by pipeline step...
static constexpr const ShaderPaths SHADERS_PATHS[] = {
    {"assets/Shaders/Background.vs", "assets/Shaders/Background.fs"},
    {"assets/Shaders/Forward.vs", "assets/Shaders/Forward.fs"},
};

/// Loads & compiles a shader.
static GLuint LoadAndCompileShader(GLenum type, const char *filepath, unsigned pass, const char *typeDescr)
{
    BOYD_LOG(Debug, "Pass {}: {}: loading from {}", pass, typeDescr, filepath);

    std::string source;
    if(!Slurp(filepath, source))
    {
        BOYD_LOG(Error, "Pass {}: {}: Could not read {}!", pass, typeDescr, filepath);
        return 0;
    }

    GLuint handle = gl3::CompileShader(type, source);
    if(handle == 0)
    {
        BOYD_LOG(Error, "Pass {}: {}: Compilation error!", pass, typeDescr);
        return 0;
    }

    return handle;
}

/// Compiles & links a shader program for a certain pass.
static SharedProgram LinkPassProgram(unsigned pass, const ShaderPaths &paths)
{
    GLuint vs = LoadAndCompileShader(GL_VERTEX_SHADER, paths.vertex, pass, "VS");
    GLuint fs = LoadAndCompileShader(GL_FRAGMENT_SHADER, paths.fragment, pass, "FS");

    SharedProgram program{};
    if(vs == 0 || fs == 0)
    {
        BOYD_LOG(Error, "Pass {}: could not compile all shaders, linking is not possible", pass);
    }
    else
    {
        program = SharedProgram{gl3::LinkProgram({vs, fs})};
        if(!program)
        {
            BOYD_LOG(Error, "Pass {}: failed to link shader program", pass);
        }
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Pipeline::Pipeline()
{
    for(unsigned iStage = 0; iStage <= _Last; iStage++)
    {
        stages[iStage].program = LinkPassProgram(iStage, SHADERS_PATHS[iStage]);
    }
}

Pipeline::~Pipeline() = default;

} // namespace gl3
} // namespace boyd
