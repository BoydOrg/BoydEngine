#pragma once

#include "../Core/Platform.hh"

namespace boyd
{
namespace comp
{

/// A "virtual component" used to tell the AssetLoader module to load a GLTF file.
/// It serves no other purpose but to be put in a ComponentLoadRequest.
struct BOYD_API Gltf
{
};

} // namespace comp
} // namespace boyd