#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include "../Gfx/Glfw.hh"

#include <algorithm>

// Here the Game state is just the InputState from the main.
// Hence, it won't be deallocated

inline boyd::InputState *GetState(void *state)
{
    return (boyd::InputState *)state;
}

extern "C" {
BOYD_API void *BoydInit_Input()
{
    BOYD_LOG(Info, "Starting the input submodule");

    auto &inputState = Boyd_GameState()->Input;
    std::fill(inputState.axes, inputState.axes + inputState.NUM_AXES, 0.0f);
    return &inputState;
}

BOYD_API void BoydUpdate_Input(void *state)
{
    auto *inputState = GetState(state);
}

BOYD_API void BoydHalt_Input(void *state)
{
    BOYD_LOG(Info, "Halting the input submodule");
}
}