#pragma once

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"

namespace boyd
{
namespace comp
{

struct BOYD_API AudioSource
{
    enum SoundType
    {
        SFX,          /// For normal sound effects that should be located in space.
        SFX_LOOPABLE, /// For looping sound effects that should be located in space.
        BGM,          /// For looping background music that should be omnidirectional.
    } soundType;

    /// Make sure those are accessible from Lua
    static constexpr int _SFX = SFX;
    static constexpr int _SFX_LOOPABLE = SFX_LOOPABLE;
    static constexpr int _BGM = BGM;

    AudioSource() = default;
    AudioSource(const AudioSource &) = default;
    AudioSource(AudioSource &&) = default;
    AudioSource &operator=(const AudioSource &) = default;
    AudioSource &operator=(AudioSource &&) = default;

    /// Force the conversion as Lua does not understand enums
    AudioSource(int soundType)
        : soundType{static_cast<SoundType>(soundType)} {}

    AudioSource(SoundType soundType)
        : soundType{soundType} {}
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::AudioSource, TRegister>
{
    static constexpr const char *TYPENAME = "AudioSource";

    static comp::AudioSource Add(comp::AudioSource *self, int type)
    {
        return {type};
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::AudioSource>(TYPENAME)
            .template addConstructor<void (*)(int)>()
            .addFunction("add", Add)
        .endClass();
        // clang-format on
    }
};

} // namespace boyd