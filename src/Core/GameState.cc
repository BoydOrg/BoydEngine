#include "GameState.hh"

extern "C" boyd::GameState *Boyd_GameState()
{
    return boyd::GameStateManager::Instance();
}
