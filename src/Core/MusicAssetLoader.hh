#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "../Components/AudioClip.hh"

namespace boyd
{

/// A general description of a header
Wave LoadWav(const std::filesystem::path &path);
} // namespace boyd
