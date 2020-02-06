#pragma once

#include "../Core/Platform.hh"
#include <raylib.h>
#include <string>

namespace boyd
{
namespace comp
{

/// A 3D transform relative to world-space
struct BOYD_API Skybox
{
    ::Model rlSkyboxModel;
    ::Mesh rlSkyboxMesh;

    Skybox(std::string hdrFilepath)
    {
        // Create Skybox mesh; set its material (shader) and texture binding points correctly
        rlSkyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
        rlSkyboxModel = LoadModelFromMesh(rlSkyboxMesh);

        rlSkyboxModel.materials[0].shader = LoadShader("assets/Shaders/Skybox.vs", "assets/Shaders/Skybox.fs");
        static constexpr const int CUBEMAP_BINDPOINT = MAP_CUBEMAP;
        SetShaderValue(rlSkyboxModel.materials[0].shader,
                       GetShaderLocation(rlSkyboxModel.materials[0].shader, "environmentMap"),
                       &CUBEMAP_BINDPOINT, UNIFORM_INT);

        // Convert HDR equirectangular map to cubemap
        Shader shdrCubeMap = LoadShader("assets/Shaders/Cubemap.vs", "assets/Shaders/Cubemap.fs");
        static constexpr const int TEXTURE_UNIT = 0;
        SetShaderValue(shdrCubeMap, GetShaderLocation(shdrCubeMap, "equirectangularMap"), &TEXTURE_UNIT, UNIFORM_INT);
        Texture2D texHDR = LoadTexture(hdrFilepath.c_str());
        rlSkyboxModel.materials[0].maps[MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubeMap, texHDR, 512);
        UnloadTexture(texHDR);
        UnloadShader(shdrCubeMap);
    }
    ~Skybox()
    {
        UnloadModel(rlSkyboxModel);
    }
};

} // namespace comp
} // namespace boyd
