#pragma once

#include "../../Components/Mesh.hh"
#include "GfxComponents.hh"
#include "Glfw.hh"

namespace boyd
{
namespace gl3
{

/// Uploads a mesh from RAM to the GPU.
/// Either generates or updates the given `gpuMesh` to match `mesh`.
/// Returns false on error.
bool UploadMesh(const comp::Mesh &mesh, GLMesh &gpuMesh);

} // namespace gl3
} // namespace boyd
