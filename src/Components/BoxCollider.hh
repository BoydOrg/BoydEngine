#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"

#include "ColliderBase.hh"
#include <fmt/format.h>

namespace boyd
{

namespace comp
{

/// A wrapper on a box collider
struct BOYD_API BoxCollider : ColliderBase
{
    float x, y, z;
    BoxCollider(float x, float y, float z)
        : x{x}, y{y}, z{z}
    {
    }

    BoxCollider() = delete;
};
} // namespace comp

template <typename TRegister>
struct Registrar<comp::BoxCollider, TRegister>
{
    constexpr static const char *TYPENAME = "BoxCollider";

    static std::string ToString(comp::BoxCollider *self)
    {
        return fmt::format(FMT_STRING("BoxCollider({} {} {})"), self->x, self->y, self->z);
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::BoxCollider>(TYPENAME)
            .template addConstructor<void(*)(float, float, float)>()
           .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};
} // namespace boyd