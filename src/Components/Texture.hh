#pragma once

#include "../Core/Platform.hh"
#include <memory>

namespace boyd
{
namespace comp
{

/// A 2D texture.
/// Not really a component, but used inside `comp::Material`s...
struct BOYD_API Texture
{
    enum Format
    {
        RGB8 = 0,
        RGBA8 = 1,
    };

    enum Usage
    {
        Static = 0,  ///< Static; load once, render many times
        Dynamic = 1, ///< Dynamic; load frequently (but not really once per frame)
        Stream = 2,  ///< Stream; load/render once per frame, discard after use
    };

    enum Filter
    {
        Nearest = 0,     ///< = GL_NEAREST
        Bilinear = 1,    ///< = GL_LINEAR
        Trilinear = 3,   ///< = GL_LINEAR_MIPMAP_LINEAR
        Anisotropic = 4, ///< = GL_LINEAR_MIPMAP_LINEAR + anisotropic filtering
    };

    struct Data
    {
        Format format{RGB8};
        unsigned width{0}, height{0};
        std::vector<uint8_t> pixels{};
        Filter minfilter{Bilinear};
        Filter magFilter{Bilinear};
        Usage usage{Static};
    };
    std::shared_ptr<Data> data;

    /// Creates a new texture given its data.
    Texture(Data &&data)
        : data{std::make_shared<Data>(std::move(data))}
    {
    }
    ~Texture() = default;
};

} // namespace comp
} // namespace boyd