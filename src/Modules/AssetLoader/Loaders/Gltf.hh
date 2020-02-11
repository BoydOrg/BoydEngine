#pragma once

#include <string>
#include <unordered_map>

#include "../../../Components/Gltf.hh"
#include "../../../Components/Material.hh"
#include "../../../Components/Mesh.hh"
#include "../LoadedAsset.hh"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>

namespace boyd
{

/// Helper to hash a pair of ints.
struct IntPairHasher
{
    inline size_t operator()(const std::pair<int, int> &key) const
    {
        return size_t((key.first * 42) ^ key.second);
    }
};

/// An atttribute in comp::Mesh::Vertex.
struct VertexAttrib
{
    size_t offset;
    size_t size;
};

/// Maps GLTF vertex attribute names to `VertexAttrib`s.
static const std::unordered_map<std::string, VertexAttrib> VERTEX_ATTRIBS{
    {"POSITION", {offsetof(comp::Mesh::Vertex, position), 3 * sizeof(float)}},
    {"NORMAL", {offsetof(comp::Mesh::Vertex, normal), 3 * sizeof(float)}},
    {"TEXCOORD_0", {offsetof(comp::Mesh::Vertex, texCoord), 2 * sizeof(float)}},
    {"COLOR_0", {offsetof(comp::Mesh::Vertex, tint), 4 * sizeof(float)}},
};

/// Maps GLTF texture filters to `comp::Texture::Filter`s.
static const std::unordered_map<int, comp::Texture::Filter> GLTF_FILTER_MAP{
    {-1, comp::Texture::Bilinear}, ///< -1: "undefined" in tinygltf -> default
    {TINYGLTF_TEXTURE_FILTER_NEAREST, comp::Texture::Nearest},
    {TINYGLTF_TEXTURE_FILTER_LINEAR, comp::Texture::Bilinear},
    {TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST, comp::Texture::Nearest},  // (no direct equivalent)
    {TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR, comp::Texture::Nearest},   // (no direct equivalent)
    {TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST, comp::Texture::Trilinear}, // (no direct equivalent)
    {TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR, comp::Texture::Trilinear},  // (no direct equivalent)
};

/// Maps GLTF <# comps, format> pairs to `comp::Texture::Format`s.
static const std::unordered_map<std::pair<int, int>, comp::Texture::Format, IntPairHasher> GLTF_FORMAT_MAP{
    // 8-bit int
    {{1, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE}, comp::Texture::R8},
    {{2, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE}, comp::Texture::RG8},
    {{3, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE}, comp::Texture::RGB8},
    {{4, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE}, comp::Texture::RGBA8},
    // 16-bit float
    {{1, TINYGLTF_COMPONENT_TYPE_SHORT}, comp::Texture::R16F},
    {{2, TINYGLTF_COMPONENT_TYPE_SHORT}, comp::Texture::RG16F},
    {{3, TINYGLTF_COMPONENT_TYPE_SHORT}, comp::Texture::RGB16F},
    {{4, TINYGLTF_COMPONENT_TYPE_SHORT}, comp::Texture::RGBA16F},
};

/// Converts a `TInteger` (pointed to by valuePtr) to a comp::mesh::Index.
template <typename TInteger>
inline static comp::Mesh::Index ConvertIndex(const void *valuePtr)
{
    return static_cast<comp::Mesh::Index>(*reinterpret_cast<const TInteger *>(valuePtr));
}

/// A map of <GLTF type, converted function to Mesh::Index>.
static const std::unordered_map<int, decltype(&ConvertIndex<int>)> INDEX_CONVERTERS{
    {TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, &ConvertIndex<uint8_t>},
    {TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, &ConvertIndex<uint16_t>},
    {TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, &ConvertIndex<uint32_t>},
};

/// Specialization of LoadedAssetBase that loads and sets multiple GLTF-loaded components.
struct LoadedGltfModel : public LoadedAssetBase
{
    std::string filepath;
    tinygltf::Model gltfModel;

    comp::Mesh mesh;
    comp::Material material;
    bool hasMesh, hasMaterial;

    LoadedGltfModel(std::string filepath, tinygltf::Model &&model)
        : LoadedAssetBase(entt::type_info<comp::Gltf>::id()), filepath{filepath}, gltfModel{std::move(model)}
    {
        // TODO IMPLEMENT: Loading for:
        // - Textures
        // - Cameras
        // - Skeletons + animations
        // - Lights
        hasMesh = hasMaterial = false;
        if(!gltfModel.meshes.empty())
        {
            if(LoadMaterial(gltfModel.meshes[0], this->material))
            {
                hasMaterial = true;
            }
            if(LoadMesh(gltfModel.meshes[0], this->mesh))
            {
                hasMesh = true;
            }
        }
    }
    ~LoadedGltfModel() = default;

    // --- Adapted from https://github.com/syoyo/tinygltf/blob/master/examples/basic/main.cpp --------------------------

    void AssignComponent(entt::registry &ecs, entt::entity target) override
    {
        if(hasMaterial)
        {
            ecs.assign_or_replace<comp::Material>(target, std::move(material));
        }
        if(hasMesh)
        {
            ecs.assign_or_replace<comp::Mesh>(target, std::move(mesh));
        }
        // GLTF loaded
        ecs.assign_or_replace<comp::Gltf>(target);
    }

private:
    bool LoadMesh(tinygltf::Mesh &gltfMesh, comp::Mesh &outMesh)
    {
        auto LoadMeshComponent = [&](comp::Mesh::Data *outMeshData) -> bool {
            for(size_t iPrimitive = 0; iPrimitive < gltfMesh.primitives.size(); iPrimitive++)
            {
                const size_t nVertsPrev = outMeshData->vertices.size(); // = number of vertices as of the last loaded primitive
                const size_t nIdxsPrev = outMeshData->indices.size();   // = number of indices as of the last loaded primitive

                const auto &primitive = gltfMesh.primitives[iPrimitive];
                if(primitive.mode != TINYGLTF_MODE_TRIANGLES)
                {
                    // TODO IMPLEMENT
                    BOYD_LOG(Warn, "{}: skipping non-triangular mesh primitive!", filepath);
                    continue;
                }

                const auto &idxAccessor = gltfModel.accessors[primitive.indices];
                const auto &idxBufferView = gltfModel.bufferViews[idxAccessor.bufferView];
                const auto &idxBuffer = gltfModel.buffers[idxBufferView.buffer];

                const size_t bytesPerIdx = tinygltf::GetComponentSizeInBytes(idxAccessor.componentType);
                const size_t nIdxsNow = nIdxsPrev + idxAccessor.count;
                outMeshData->indices.resize(nIdxsNow);
                const auto idxConv = INDEX_CONVERTERS.at(idxAccessor.componentType);

                const uint8_t *readPtr = &idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset];
                for(size_t iIdx = nIdxsPrev; iIdx < nIdxsNow; iIdx++)
                {
                    // Shift all indices by the previous primitive's vertex count to merge the two...
                    outMeshData->indices[iIdx] = idxConv(readPtr) + nVertsPrev;
                    readPtr += bytesPerIdx;
                }

                for(const auto &attribPair : primitive.attributes)
                {
                    // Find vertex attribute by name...
                    auto vertAttribIt = VERTEX_ATTRIBS.find(attribPair.first);
                    if(vertAttribIt == VERTEX_ATTRIBS.end())
                    {
                        // Not an attribute we are interested in
                        continue;
                    }
                    const VertexAttrib &vertAttrib = vertAttribIt->second;

                    const auto &attribAccessor = gltfModel.accessors[attribPair.second];
                    if(attribAccessor.sparse.isSparse)
                    {
                        // TODO IMPLEMENT
                        BOYD_LOG(Warn, "{}: sparse buffer view is not supported", filepath);
                        continue;
                    }

                    // FIXME: Check that the component type == GL_FLOAT!
                    //        (it should be, but you never know...)
                    const auto gltfCompSize = static_cast<size_t>(tinygltf::GetComponentSizeInBytes(attribAccessor.componentType));
                    const auto gltfCompCount = static_cast<size_t>(tinygltf::GetNumComponentsInType(attribAccessor.type));
                    const auto gltfAttribSize = gltfCompSize * gltfCompCount;
                    if(gltfAttribSize != vertAttrib.size)
                    {
                        BOYD_LOG(Warn, "{}: GLTF attribute {}'s size is ({}x{})B, but in Mesh it's {}B - truncating!",
                                 filepath, attribPair.first, gltfCompSize, gltfCompCount, vertAttrib.size);
                    }
                    const size_t attribSize = std::min(gltfAttribSize, vertAttrib.size);

                    const auto &attribBufferView = gltfModel.bufferViews[attribAccessor.bufferView];
                    const auto &attribBuffer = gltfModel.buffers[attribBufferView.buffer];

                    // Ensure there are enough vertices to store all attribute values for this primitive
                    outMeshData->vertices.resize(nVertsPrev + attribAccessor.count);

                    const uint8_t *readPtr = &attribBuffer.data[attribBufferView.byteOffset + attribAccessor.byteOffset];
                    uint8_t *writePtr = reinterpret_cast<uint8_t *>(&outMeshData->vertices[nVertsPrev]) + vertAttrib.offset;
                    for(size_t iVertex = 0; iVertex < attribAccessor.count; iVertex++)
                    {
                        memcpy(writePtr, readPtr, attribSize);
                        writePtr += sizeof(comp::Mesh::Vertex);
                        readPtr += gltfAttribSize;
                    }
                }
            }

            return true; // comp::Mesh::Data was edited
        };
        outMesh.data.Edit(LoadMeshComponent);

        return true;
    }

    bool LoadTexture(int gltfTextureId, comp::Texture &outTexture)
    {
        const auto &gltfTexture = gltfModel.textures[gltfTextureId];
        const auto &gltfImage = gltfModel.images[gltfTexture.source];

        comp::Texture::Filter minFilter = comp::Texture::Bilinear, magFilter = comp::Texture::Bilinear;
        if(gltfTexture.sampler >= 0)
        {
            const auto &gltfSampler = gltfModel.samplers[gltfTexture.sampler];
            minFilter = GLTF_FILTER_MAP.at(gltfSampler.minFilter);
            magFilter = GLTF_FILTER_MAP.at(gltfSampler.magFilter);
        }

        const auto formatIt = GLTF_FORMAT_MAP.find({gltfImage.component, gltfImage.pixel_type});
        if(formatIt == GLTF_FORMAT_MAP.end())
        {
            BOYD_LOG(Error, "Unsupporte texture format for texture {} - it will be ignored!", gltfTextureId);
        }

        // TODO IMPLEMENT image format conversion?

        outTexture = comp::Texture{comp::Texture::Data{
            formatIt->second,
            unsigned(gltfImage.width),
            unsigned(gltfImage.height),
            gltfImage.image, // (copy)
            minFilter,
            magFilter,
            comp::Texture::Static,
        }};
        return true;
    }

    bool LoadMaterial(tinygltf::Mesh &gltfMesh, comp::Material &outMaterial)
    {
        if(gltfMesh.primitives.empty())
        {
            return false;
        }
        const auto &gltfPrimitive = gltfMesh.primitives[0];
        const auto &gltfMaterial = gltfModel.materials[gltfPrimitive.material];

        // TODO: This only loads the first primitive's material...
        // TODO: Also load the color multipliers for the textures?
        // TODO: Load other textures?

        int gltfDiffuseTextureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;

        comp::Texture diffuseMap{comp::Texture::Data{
            comp::Texture::RGB8,
            1,
            1,
            {255, 255, 255},
            comp::Texture::Nearest,
            comp::Texture::Nearest,
            comp::Texture::Static,
        }};

        if(gltfDiffuseTextureIndex >= 0)
        {
            const auto &gltfDiffuseTexture = gltfModel.textures[gltfDiffuseTextureIndex];
            if(!LoadTexture(gltfDiffuseTexture.source, diffuseMap))
            {
                BOYD_LOG(Error, "Failed to load diffuse map!");
            }
        }
        // else: default to the white texture

        outMaterial.parameters["DiffuseMap"] = std::move(diffuseMap);

        return true;
    }
};

template <>
struct Loader<comp::Gltf>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        tinygltf::TinyGLTF gltf;
        tinygltf::Model model;
        bool ok = false;

        std::string err, warn;
        ok = gltf.LoadBinaryFromFile(&model, &err, &warn, filepath);
        if(!warn.empty())
        {
            BOYD_LOG(Warn, "{}: {}", filepath, err);
        }
        if(!err.empty())
        {
            BOYD_LOG(Error, "{}: {}", filepath, err);
        }

        if(ok)
        {
            return std::make_unique<LoadedGltfModel>(filepath, std::move(model));
        }
        else
        {
            return nullptr;
        }
    }
};

} // namespace boyd
