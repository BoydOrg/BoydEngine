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
void PrintOpenALError();

void PrintOpenALCError(ALCdevice* device);
}