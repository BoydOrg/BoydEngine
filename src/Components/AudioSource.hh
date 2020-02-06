#pragma once

#include "../Core/Platform.hh"

namespace boyd
{
namespace comp
{

struct BOYD_API AudioSource
{
    enum class SoundType
    {
        SFX,          /// For normal sound effects that should be located in space.
        SFX_LOOPABLE, /// For looping sound effects that should be located in space.
        BGM,          /// For looping background music that should be omnidirectional.
    } soundType;
};

} // namespace comp
} // namespace boyd