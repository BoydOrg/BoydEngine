#pragma once

#include <string>
#include <unordered_map>

#include "../../../Components/Gltf.hh"
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
    {"COLOR_0", {offsetof(comp::Mesh::Vertex, tintEmission), 3 * sizeof(float)}},
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
    bool hasMesh;

    LoadedGltfModel(std::string filepath, tinygltf::Model &&model)
        : LoadedAssetBase(entt::type_info<comp::Gltf>::id()), filepath{filepath}, gltfModel{std::move(model)}
    {
        // TODO IMPLEMENT: Loading for:
        // - Textures
        // - Cameras
        // - Skeletons + animations
        // - Lights
        hasMesh = false;
        if(!gltfModel.meshes.empty())
        {
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
        if(hasMesh)
        {
            ecs.assign_or_replace<comp::Mesh>(target, std::move(mesh));
        }
        ecs.assign_or_replace<comp::Gltf>(target);
    }

private:
    bool LoadMesh(tinygltf::Mesh &gltfMesh, comp::Mesh &outMesh)
    {
        for(size_t iPrimitive = 0; iPrimitive < gltfMesh.primitives.size(); iPrimitive++)
        {
            const size_t nVertsPrev = outMesh.data->vertices.size(); // = number of vertices as of the last loaded primitive
            const size_t nIdxsPrev = outMesh.data->indices.size();   // = number of indices as of the last loaded primitive

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
            outMesh.data->indices.resize(nIdxsNow);
            const auto idxConv = INDEX_CONVERTERS.at(idxAccessor.componentType);

            const uint8_t *readPtr = &idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset];
            for(size_t iIdx = nIdxsPrev; iIdx < nIdxsNow; iIdx++)
            {
                // Shift all indices by the previous primitive's vertex count to merge the two...
                outMesh.data->indices[iIdx] = idxConv(readPtr) + nVertsPrev;
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
                outMesh.data->vertices.resize(nVertsPrev + attribAccessor.count);

                const uint8_t *readPtr = &attribBuffer.data[attribBufferView.byteOffset + attribAccessor.byteOffset];
                uint8_t *writePtr = reinterpret_cast<uint8_t *>(&outMesh.data->vertices[nVertsPrev]) + vertAttrib.offset;
                for(size_t iVertex = 0; iVertex < attribAccessor.count; iVertex++)
                {
                    memcpy(writePtr, readPtr, attribSize);
                    writePtr += sizeof(comp::Mesh::Vertex);
                    readPtr += gltfAttribSize;
                }
            }
        }

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
