#pragma once

#include "../Core/Platform.hh"
#include "../Core/Versioned.hh"

namespace boyd
{
namespace comp
{

/// A 2D texture.
/// Not really a component, but used inside `comp::Material`s...
struct BOYD_API Texture
{
    enum Type
    {
        T2D,      ///< Standard; `width` * `height`.
        T3D,      ///< 3D texture; `width` * `height` * `depth`.
        T2DArray, ///< 2D texture array; `depth` layers of `width` * `height` textures.
        TCubemap, ///< Cubemap texture; 6 `width`^2 textures (in order: +X [right], -X [left], +Y [top], -Y [bottom], +Z [back], -Z [front]).
    };

    enum Format
    {
        // 8-bit unsigned normalized
        R8,
        RG8,
        RGB8,
        RGBA8,
        // 16-bit float
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
    };

    enum Usage
    {
        Static = 0, ///< Static; load once, render many times
        Dynamic,    ///< Dynamic; load frequently (but not really once per frame)
        Stream,     ///< Stream; load/render once per frame, discard after use
    };

    enum Filter
    {
        Nearest = 0, ///< = GL_NEAREST
        Bilinear,    ///< = GL_LINEAR
        Trilinear,   ///< = GL_LINEAR_MIPMAP_LINEAR
        Anisotropic, ///< = GL_LINEAR_MIPMAP_LINEAR + anisotropic filtering
    };

    struct Data
    {
        Type type{T2D};
        Format format{RGB8};
        unsigned width{0}, height{0}, depth{0};
        std::vector<uint8_t> pixels{};
        Filter minFilter{Bilinear};
        Filter magFilter{Bilinear}; ///< NOTE: Only accepts Nearest or Bilinear!
        Usage usage{Static};
    };
    Versioned<Data> data;

    /// Creates a new texture given its data.
    Texture(Data &&data)
        : data{Versioned<Data>::Make(data)}
    {
    }
    ~Texture() = default;
};

} // namespace comp
} // namespace boyd
