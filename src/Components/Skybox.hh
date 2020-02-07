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
    std::string skyboxAsset;

    ::Model raylibSkyboxModel;
    ::Mesh raylibSkyboxMesh;

    /// Init a Skydome
    Skybox(std::string skyboxAsset)
        : skyboxAsset{skyboxAsset}
    {

        // Get a dome image as an equirectangular map
        // Run a shader to
        raylibSkyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
        raylibSkyboxModel = LoadModelFromMesh(raylibSkyboxMesh);
        raylibSkyboxModel.materials[0].shader = LoadShader("assets/Shaders/Skybox.vs", "assets/Shaders/Skybox.fs");

        auto uniformValue = MAP_CUBEMAP;
        auto targetTexture = 0;
        SetShaderValue(raylibSkyboxModel.materials[0].shader,
                       GetShaderLocation(raylibSkyboxModel.materials[0].shader, "environmentMap"),
                       &uniformValue, UNIFORM_INT);

        Shader shdrCubeMap = LoadShader("assets/Shaders/Cubemap.vs", "assets/Shaders/Cubemap.fs");
        SetShaderValue(shdrCubeMap, GetShaderLocation(shdrCubeMap, "equirectangularMap"), &targetTexture, UNIFORM_INT);
        Texture2D texHDR = LoadTexture(skyboxAsset.c_str());

        raylibSkyboxModel.materials[0].maps[MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubeMap, texHDR, 512);
        UnloadTexture(texHDR);
        UnloadShader(shdrCubeMap);
    }
};

} // namespace comp
} // namespace boyd