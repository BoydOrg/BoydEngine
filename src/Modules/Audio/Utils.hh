#include <AL/alc.h>

namespace boyd
{

/// -- Do not use this directly! Use the macro BOYD_OPENAL_ERROR instead. --
void PrintOpenALError(const char *file, int line);

/// -- Do not use this directly! Use the macro BOYD_OPENALC_ERROR instead. --
void PrintOpenALCError(ALCdevice *device, const char *file, int line);
} // namespace boyd

/// Log an OpenAL error, if any
#define BOYD_OPENAL_ERROR() boyd::PrintOpenALError(__FILE__, __LINE__);

/// Log an OpenALC error, if any
#define BOYD_OPENALC_ERROR(device) boyd::PrintOpenALCError(device, __FILE__, __LINE__);
