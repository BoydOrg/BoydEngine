#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"

#include "../../Debug/Log.hh"

#include <AL/al.h>
#include <AL/alc.h>
#include <tinywav.h>

using namespace boyd;

struct BoydAudioState
{
    ALCdevice *device{nullptr};
    ALCcontext *context{nullptr};
    bool isEnabled{true};

    BoydAudioState()
    {
        if(!(device = alcOpenDevice(nullptr)))
        {
            BOYD_LOG(Error, "Could not open a default speaker. Audio will be muted.");
            isEnabled = false;
        }
        if(!(context = alcCreateContext(device, nullptr)))
        {
            BOYD_LOG(Error, "Could not instantiate an OpenAL context. Audio will be muted.");
            isEnabled = false;
        }
    }
};

inline BoydAudioState *GetState(void *state)
{
    return reinterpret_cast<BoydAudioState *>(state);
}

extern "C" {
BOYD_API void *BoydInit_Audio()
{
    BOYD_LOG(Info, "Starting Audio module");
    return new BoydAudioState;
}

BOYD_API void BoydUpdate_Audio(void *state)
{
    BOYD_LOG(Info, "Update2...");
    boyd::GameState *gameState = Boyd_GameState();
}

BOYD_API void BoydHalt_Audio(void *state)
{
    BOYD_LOG(Info, "Halting AudioState module");
    delete GetState(state);
}
}