#include "../../Components/AudioSource.hh"
#include <filesystem>

namespace boyd
{
/// Load a RIFF wav file.
/// `path` - the asset directory
/// `audioSource` - the AudioSource component
void LoadWav(const std::filesystem::path &path,
             boyd::comp::AudioSource &audioSource);
/// Load a FLAC file.
/// `path` - the asset directory
/// `audioSource` - the AudioSource component
void LoadFlac(const std::filesystem::path &path,
              boyd::comp::AudioSource &audioSource);

/// Fetches the internal OpenALGet error
void PrintOpenALError(const char *file, int line);

void PrintOpenALCError(ALCdevice *device, const char *file, int line);
} // namespace boyd

/// This is meant to be a wrapper that also prints out the line where the issue was raised.
/// TODO: Those two calls are suboptimal indeed.
/// it would be better to just invoke the logging manager with different parameters.
#define BOYD_OPENAL_ERROR() \
    boyd::PrintOpenALError(__FILE__, __LINE__);

/// Similra for BOYD_OPENAL_ERROR() but specific to OpenALC
#define BOYD_OPENALC_ERROR(device) \
    boyd::PrintOpenALCError(device, __FILE__, __LINE__);
