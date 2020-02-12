#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"
#include <string>

namespace boyd
{
namespace comp
{

/// A simple string, usable as a component.
struct BOYD_API LuaBehaviour
{
    std::string source;
    std::string description;

    LuaBehaviour(std::string value)
        : source{value}
    {
    }
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::LuaBehaviour, TRegister>
{
    static constexpr const char *TYPENAME = "LuaBehaviour";

    static std::string ToString(const comp::LuaBehaviour *self)
    {
        return self->source;
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::LuaBehaviour>(TYPENAME)
            .template addConstructor<void(*)(std::string)>()
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd
