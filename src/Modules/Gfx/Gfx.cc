#include "Gfx.hh"

#include <entt/entt.hpp>

#include "../../Components/Camera.hh"
#include "../../Components/Material.hh"
#include "../../Components/Mesh.hh"
#include "../../Components/Skybox.hh"
#include "../../Components/Transform.hh"

#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
using namespace boyd;

#include "GL3/GL3.hh"
#include "Glfw.hh"

namespace boyd
{

bool BoydGfxState::InitContext()
{
    BOYD_LOG(Debug, "Initializing GLFW");
    if(!glfwInit())
    {
        const char *error;
        int errorCode = glfwGetError(&error);
        BOYD_LOG(Error, "Failed to init GLFW: [{}] {}", errorCode, error);
        return false;
    }

    BOYD_LOG(Debug, "Creating GLFW window");

    glfwWindowHint(GLFW_RESIZABLE, true);
    // Request OpenGL ES 3
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(800, 600, "BoydEngine", nullptr, nullptr);
    if(!window)
    {
        const char *error;
        int errorCode = glfwGetError(&error);
        BOYD_LOG(Error, "Failed to create GLFW window: [{}] {}", errorCode, error);
        return false;
    }
    glfwMakeContextCurrent(window);

    BOYD_LOG(Debug, "OpenGL: {} ({})", glGetString(GL_VERSION), glGetString(GL_VENDOR));

#ifdef BOYD_GLES3_FLEXTGL
    if(!flextInit(window))
    {
        BOYD_LOG(Error, "flextGL failed!");
        return false;
    }
#endif

    return true;
}

gl3::SharedTexture BoydGfxState::MapGpuTexture(const comp::Texture &texture)
{
    auto gpuTextureIt = textureMap.find(texture.data);
    bool needsUploading = false;
    if(gpuTextureIt == textureMap.end())
    {
        // Texture is not on GPU -> Add a entry for it and mark it as to be uploaded (current version from RAM to VRAM!)
        gpuTextureIt = textureMap.emplace(texture.data, std::make_pair(gl3::SharedTexture{0}, texture.data.Version())).first;
        needsUploading = true;
    }
    else if(gpuTextureIt->second.second < texture.data.Version())
    {
        // Texture on GPU is outdated -> Need to upload the modifications
        needsUploading = true;
    }

    if(needsUploading)
    {
        bool uploadOk = gl3::UploadTexture(texture, gpuTextureIt->second.first);
        if(!uploadOk)
        {
            BOYD_LOG(Warn, "Failed to upload texture to GPU");
            return gl3::SharedTexture{0};
        }
    }

    return gpuTextureIt->second.first;
}

gl3::SharedMesh BoydGfxState::MapGpuMesh(const comp::Mesh &mesh)
{
    auto gpuMeshIt = meshMap.find(mesh.data);
    bool needsUploading = false;
    if(gpuMeshIt == meshMap.end())
    {
        // Mesh is not on GPU -> Add a entry for it and mark it as to be uploaded (current version from RAM to VRAM!)
        gpuMeshIt = meshMap.emplace(mesh.data, std::make_pair(gl3::SharedMesh{}, mesh.data.Version())).first;
        needsUploading = true;
    }
    else if(gpuMeshIt->second.second < mesh.data.Version())
    {
        // Mesh on GPU is outdated -> Need to upload the modifications
        needsUploading = true;
    }

    if(needsUploading)
    {
        bool uploadOk = gl3::UploadMesh(mesh, gpuMeshIt->second.first);
        if(!uploadOk)
        {
            BOYD_LOG(Warn, "Failed to upload mesh to GPU");
            return gl3::SharedMesh{};
        }
    }

    return gpuMeshIt->second.first;
}

unsigned BoydGfxState::ApplyMaterialParams(const comp::Material &material, gl3::SharedProgram &program)
{
    std::string uniformName;
    unsigned nTexturesApplied = 0;

    for(const auto &param : material.parameters)
    {
        uniformName = fmt::format(FMT_STRING("u_{}"), param.first);
        auto uniformLocIt = program.uniforms.find(uniformName);
        if(uniformLocIt == program.uniforms.end())
        {
            continue;
        }
        GLint uniformLoc = uniformLocIt->second;

        switch(param.second.index())
        {
        case 0: // float
            glUniform1f(uniformLoc, std::get<float>(param.second));
            break;
        case 1: // glm::vec2
            glUniform2fv(uniformLoc, 1, &std::get<glm::vec2>(param.second)[0]);
            break;
        case 2: // glm::vec3
            glUniform3fv(uniformLoc, 1, &std::get<glm::vec3>(param.second)[0]);
            break;
        case 3: // glm::vec4
            glUniform4fv(uniformLoc, 1, &std::get<glm::vec4>(param.second)[0]);
            break;
        case 4: // glm::mat3
            glUniformMatrix3fv(uniformLoc, 1, false, &std::get<glm::mat3>(param.second)[0][0]);
            break;
        case 5: // glm::mat4
            glUniformMatrix4fv(uniformLoc, 1, false, &std::get<glm::mat4>(param.second)[0][0]);
            break;
        case 6: // comp::Texture
        {
            auto gpuTexture = MapGpuTexture(*std::get_if<comp::Texture>(&param.second));
            glActiveTexture(GL_TEXTURE0 + nTexturesApplied);
            glBindTexture(GL_TEXTURE_2D, gpuTexture); // TODO: support non-2D textures?

            glUniform1i(uniformLoc, nTexturesApplied); // Bind sampler to texture unit

            nTexturesApplied++;
        }
        break;
        default:
            // *X-Files main theme*
            break;
        }
    }

    return nTexturesApplied;
}

void BoydGfxState::Update()
{
    auto *gameState = Boyd_GameState();

    if(!window)
    {
        // Can't to much without a window
        return;
    }
    int screenW, screenH;
    glfwGetFramebufferSize(window, &screenW, &screenH);
    glm::vec2 screenSize{screenW, screenH};

    // TODO: Need a way to actually pick what camera to use - find the camera tagged "MainCamera"?
    auto cameraView = gameState->ecs.view<comp::Camera, comp::ActiveCamera>();

    entt::entity cameraEntity = entt::null;
    glm::mat4 projMtx, viewMtx;

    if(!cameraView.empty())
    {
        cameraEntity = *cameraView.begin();
        const auto &camera = gameState->ecs.get<comp::Camera>(cameraEntity);

        switch(camera.mode)
        {
        case comp::Camera::Persp:
            projMtx = glm::perspectiveFov(camera.fov, screenSize.x, screenSize.y, camera.zNear, camera.zFar);
            break;
        default: // comp::Camera::Ortho
            if(glm::isinf(camera.zNear) || glm::isinf(camera.zFar))
            {
                projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top);
            }
            else
            {
                projMtx = glm::ortho(camera.left, camera.right, camera.bottom, camera.top, camera.zNear, camera.zFar);
            }
            break;
        }

        viewMtx = glm::identity<glm::mat4>();
        if(gameState->ecs.has<comp::Transform>(cameraEntity))
        {
            viewMtx = gameState->ecs.get<comp::Transform>(cameraEntity).matrix;
            viewMtx = glm::inverse(viewMtx); // Inverse, because this is a camera view matrix!
        }
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, screenW, screenH);

    if(gameState->ecs.valid(cameraEntity))
    {
        glm::mat4 viewProjectionMtx = projMtx * viewMtx;

        // -------------------------------------------------------------------------------------------------------------
        // Forward pass: render all meshes in the ECS with forward lighting
        // -------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        auto &stage = pipeline->stages[gl3::Pipeline::Forward];

        glUseProgram(stage.program);

        unsigned nTextures = 0; // Number of textures bound the previous drawcall
        gameState->ecs.view<comp::Transform, comp::Mesh, comp::Material>()
            .each([&](auto entity, const comp::Transform &transform, const comp::Mesh &mesh, const comp::Material &material) {
                const auto gpuMesh = MapGpuMesh(mesh);

                // Apply uniforms + bind the textures needed for this drawcall
                // (uploads textures to VRAM if they weren't already there)
                unsigned nTexturesNow = ApplyMaterialParams(material, stage.program);

                glm::mat4 mvpMtx = viewProjectionMtx * transform.matrix;
                glUniformMatrix4fv(stage.program.uniforms["u_ModelViewProjection"], 1, false, &mvpMtx[0][0]);

                // Unbind all textures that would be unused this drawcall
                for(unsigned i = nTextures; i > nTexturesNow; i--)
                {
                    glActiveTexture(GL_TEXTURE0 + i - 1);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                nTextures = nTexturesNow;

                // Bind VBO+IBO and render
                glBindVertexArray(gpuMesh.vao);
                glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, nullptr);
            });

        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_DEPTH_TEST);
    }
    // else: Hard to render anything without a camera...

    // -------------------------------------------------------------------------

    glfwSwapBuffers(window);

    // Poll input at end of frame to minimize delay between the Gfx system running (that should be the last one in the sequence)
    // and the next frame
    UpdateInput(this);

    if(glfwWindowShouldClose(window))
    {
        gameState->running = false;
    }
}

} // namespace boyd

inline BoydGfxState *GetState(void *state)
{
    return reinterpret_cast<boyd::BoydGfxState *>(state);
}

extern "C" {

BOYD_API void *BoydInit_Gfx()
{
    BOYD_LOG(Info, "Starting Gfx module");
    auto *state = new BoydGfxState;
    InitInput(state);
    return state;
}

BOYD_API void BoydUpdate_Gfx(void *statePtr)
{
    auto *gfxState = GetState(statePtr);
    gfxState->Update();
}

BOYD_API void BoydHalt_Gfx(void *state)
{
    BOYD_LOG(Info, "Halting Gfx module");
    delete GetState(state);
}
}
