#pragma once

#include "../Core/Platform.hh"
#include <PolyVox/RawVolume.h>
#include <cstdint>
#include <memory>

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

    std::unique_ptr<Volume> volume;

    Voxels(int32_t width, int32_t height, int32_t depth)
    {
        volume = std::make_unique<Volume>(PolyVox::Region({0, 0, 0}, {width, height, depth}));
    }
    ~Voxels() = default;

    Voxels(const Voxels &toCopy) = delete;
    Voxels &operator=(const Voxels &toCopy) = delete;
    Voxels(Voxels &&toMove) = default;
    Voxels &operator=(Voxels &&toMove) = default;
};

/// Marks a `Voxels` component in the same entity as "dirty" (i.e., need remeshing)
struct BOYD_API VoxelsDirty
{
};

} // namespace comp
} // namespace boyd
