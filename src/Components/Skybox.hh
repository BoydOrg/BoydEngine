#pragma once

#include "../Core/Platform.hh"

namespace boyd
{
namespace comp
{

/// A renderable skybox.
/// This is a "virtual component"; attach a `ComponentLoadRequest<comp::Skybox>`
/// (pointing to the directory containing the skybox HDRs) for it to be loaded.
struct BOYD_API Skybox
{
};

} // namespace comp
} // namespace boyd
