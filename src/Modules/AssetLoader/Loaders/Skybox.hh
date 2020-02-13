#pragma once

#include <fmt/format.h>
#include <string.h>
// NOTE: <stb_image.h> is already included from tinygltf's source tree; re #including it causes errors, so not doing it!
#include "../../../Components/Material.hh"
#include "../../../Components/Skybox.hh"
#include "../../../Debug/Log.hh"
#include "../LoadedAsset.hh"

namespace boyd
{

/// Specialization of LoadedAssetBase that loads and sets a cubemap texture as a Material parameter.
struct LoadedSkybox : public LoadedAssetBase
{
    comp::Texture texture;

    LoadedSkybox(comp::Texture &&texture)
        : LoadedAssetBase(entt::type_info<comp::Skybox>::id()), texture{std::move(texture)}
    {
    }
    ~LoadedSkybox() = default;

    void AssignComponent(entt::registry &ecs, entt::entity target) override
    {
        if(!ecs.valid(target))
        {
            // Target is kil
            return;
        }

        ecs.get_or_assign<comp::Material>(target).parameters["EnvMap"] = texture;
        // Skybox loaded
        ecs.assign_or_replace<comp::Skybox>(target);
    }
};

template <>
struct Loader<comp::Skybox>
{
    static constexpr const char *FACE_NAMES[6] = {
        // (in order of appearance in a skybox texture)
        "px",
        "nx",
        "py",
        "ny",
        "pz",
        "nz",
    };

    static std::unique_ptr<LoadedAssetBase> Load(std::string dirpath)
    {
        // NOTE: Filepath for Skybox is the path to the directory where the ".hdr"s are
        unsigned size = 0;
        std::vector<uint8_t> data{};

        bool failed = false;
        for(unsigned i = 0; i < 6 && !failed; i++)
        {
            std::string filepath = fmt::format(FMT_STRING("{}/{}.hdr"), dirpath, FACE_NAMES[i]);

            int width = 0, height = 0, nComponents = 0;
            float *faceData = stbi_loadf(filepath.c_str(), &width, &height, &nComponents, 3);
            if(!faceData)
            {
                BOYD_LOG(Error, "{}: Failed to load file", filepath);
                failed = true;
                break;
            }
            if(width != height)
            {
                BOYD_LOG(Error, "{}: Skybox face is not square ({}x{})", filepath, width, height);
                failed = true;
                break;
            }
            if(i == 0)
            {
                size = width;
            }
            else if(size != width)
            {
                BOYD_LOG(Error, "{0}: Skybox face is not of the right size (expected {1}x{1}, got {2}x{2})", filepath, size, width);
                failed = true;
                break;
            }

            auto rawDataStart = reinterpret_cast<const uint8_t *>(faceData);
            auto rawDataEnd = rawDataStart + size * size * sizeof(float);
            data.insert(data.end(), rawDataStart, rawDataEnd);

            stbi_image_free(faceData);
        }

        if(failed)
        {
            BOYD_LOG(Error, "Skybox at {} failed to load", dirpath);
            return nullptr;
        }

        comp::Texture texture{comp::Texture::Data{
            comp::Texture::TCubemap,
            comp::Texture::RGB16F,
            size,
            size,
            0,
            std::move(data),
            comp::Texture::Bilinear,
            comp::Texture::Bilinear,
            comp::Texture::Static,
        }};
        return std::make_unique<LoadedSkybox>(std::move(texture));
    }
};

} // namespace boyd
