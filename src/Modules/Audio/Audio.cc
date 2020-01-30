#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

/// TODO: add state transfer
struct BoydAudioState {
    /// TODO: add raylib core stuff...
};

inline BoydAudioState* GetState(void* state)
{
    return reinterpret_cast<BoydAudioState*>(state);
}


extern "C" {
    BOYD_API void* BoydInit_Audio()
    {
        BOYD_LOG(Info, "Starting Audio module");
        return new BoydAudioState;
    }

    BOYD_API void BoydUpdate_Audio(void* state)
    {
        BOYD_LOG(Info, "Update2...");
    }

    BOYD_API void BoydHalt_Audio(void* state)
    {
        BOYD_LOG(Info, "Halting AudioState module");
        delete GetState(state);
    }
}