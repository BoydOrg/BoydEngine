#pragma once

namespace boyd
{

/// Helper template used to register a type `T` into a `TRegister`.
template <typename T, typename TRegister>
struct Registrar
{
    /// The typename of the `T` (as it is to be registered).
    static constexpr const char *TYPENAME = nullptr;

    /// Registers a type `T` for use into the given register of type `TRegister`.
    static void Register(TRegister &reg);
};

} // namespace boyd