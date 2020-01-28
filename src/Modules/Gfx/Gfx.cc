#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

/// TODO: add state transfer
struct BoydGfxState {
    /// TODO: add raylib core stuff...
};

inline BoydGfxState* GetState(void* state)
{
    return reinterpret_cast<BoydGfxState*>(state);
}


extern "C" {
    BOYD_API void* BoydInit_Gfx()
    {
        BOYD_LOG(Info, "Starting Gfx module");
        return new BoydGfxState;
    }

    BOYD_API void BoydUpdate_Gfx(void* state)
    {
        BOYD_LOG(Info, "Update2...");
    }

    BOYD_API void BoydHalt_Gfx(void* state)
    {
        BOYD_LOG(Info, "Halting Gfx module");
        delete GetState(state);
    }
}