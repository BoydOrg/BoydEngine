#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"
#include <string>

namespace boyd
{
namespace comp
{

/// A simple string, usable as a component.
struct BOYD_API String
{
    std::string string;

    String(std::string value)
        : string{value}
    {
    }
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::String, TRegister>
{
    static constexpr const char *TYPENAME = "String";

    static std::string ToString(const comp::String *self)
    {
        return self->string;
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::String>(TYPENAME)
            .template addConstructor<void(*)(std::string)>()
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd
