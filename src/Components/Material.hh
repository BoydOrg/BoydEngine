#pragma once

#include "../Core/Platform.hh"
#include <raylib.h>

using RaylibMaterial = Material;
namespace boyd
{
namespace comp
{
struct BOYD_API Material
{
    RaylibMaterial material;
    bool isDiffuseMap;
};
} // namespace comp
} // namespace boyd