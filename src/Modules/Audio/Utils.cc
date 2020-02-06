#include "Utils.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include <AL/al.h>
#include <AL/alc.h>

namespace boyd
{
void PrintOpenALError(const char *file, int line)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        boyd::Log::instance().log(boyd::LogLevel::Warn, &file[BoydEngine__FILE__OFFSET], line, "{}", alGetString(error));
    }
}

void PrintOpenALCError(ALCdevice *device, const char *file, int line)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
    {
        boyd::Log::instance().log(boyd::LogLevel::Warn, &file[BoydEngine__FILE__OFFSET], line, "{}", alcGetString(device, error));
    }
}
} // namespace boyd