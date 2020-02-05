#pragma once

#include <PolyVox/RawVolume.h>
#include <cstdint>
#include <fmt/format.h>
#include <memory>

#include "../Core/Platform.hh"
#include "../Core/Registrar.hh"

namespace boyd
{
namespace comp
{

/// The type of a single voxel.
using Voxel = uint8_t;

/// A voxel model.
struct BOYD_API Voxels
{
    using Volume = PolyVox::RawVolume<Voxel>;

    std::shared_ptr<Volume> volume;

    Voxels(int32_t width, int32_t height, int32_t depth)
    {
        volume = std::make_unique<Volume>(PolyVox::Region({0, 0, 0}, {width, height, depth}));
    }
    ~Voxels() = default;

    // NOTE: All components exposed to the scripting API must also be copy-constructible!
    Voxels(const Voxels &toCopy) = default;
    Voxels &operator=(const Voxels &toCopy) = default;
    Voxels(Voxels &&toMove) = default;
    Voxels &operator=(Voxels &&toMove) = default;
};

/// Marks a `Voxels` component in the same entity as "dirty" (i.e., need remeshing)
struct BOYD_API VoxelsDirty
{
};

} // namespace comp

template <typename TRegister>
struct Registrar<comp::Voxels, TRegister>
{
    static constexpr const char *TYPENAME = "Voxels";

    static comp::Voxel GetVoxel(const comp::Voxels *self, int32_t x, int32_t y, int32_t z)
    {
        // NOTE: polyVox already does bounds-checking internally, returns "borderValue" on out-of-bounds
        return self->volume->getVoxel(x, y, z);
    }
    static bool SetVoxel(comp::Voxels *self, int32_t x, int32_t y, int32_t z, comp::Voxel value)
    {
        // NOTE: Returns true on success or false otherwise (out-of-bounds)

        // (polyVox already does bounds-checking internally)
        if(!self->volume->getEnclosingRegion().containsPoint(x, y, z))
        {
            return false;
        }
        self->volume->setVoxel(x, y, z, value);
        return true;
    }
    static std::string ToString(const comp::Voxels *self)
    {
        return fmt::format(FMT_STRING("Voxels(w={}, h={}, d={})"),
                           self->volume->getWidth(), self->volume->getHeight(), self->volume->getDepth());
    }

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::Voxels>(TYPENAME)
            .template addConstructor<void(*)(int32_t, int32_t, int32_t)>()
            .addFunction("getvoxel", &GetVoxel)
            .addFunction("setvoxel", SetVoxel)
            .addFunction("__tostring", ToString)
        .endClass();
        // clang-format on
    }
};

template <typename TRegister>
struct Registrar<comp::VoxelsDirty, TRegister>
{
    static constexpr const char *TYPENAME = "VoxelsDirty";

    static TRegister Register(TRegister &reg)
    {
        // clang-format off
        return reg.template beginClass<comp::VoxelsDirty>(TYPENAME)
            .template addConstructor<void(*)(void)>()
        .endClass();
        // clang-format on
    }
};

} // namespace boyd
