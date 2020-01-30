#pragma once

#include "Platform.hh"
#include <entt/entt.hpp>

namespace boyd
{

/// The whole game state, shared among modules.
struct BOYD_API GameState
{
    entt::registry ecs; ///< The EnTT ECS.
};

/// A singleton used to manage a `GameState`.
class BOYD_API GameStateManager
{
private:
    GameStateManager()
    {
        state = new GameState();
    }

public:
    static GameStateManager &Instance()
    {
        static GameStateManager inst;
        return inst;
    }

    ~GameStateManager()
    {
        delete state;
        state = nullptr;
    }

    inline operator GameState *()
    {
        return state;
    }

private:
    GameState *state;
};

} // namespace boyd

/// Gets a pointer to the global GameState (that is shared among modules).
extern "C" BOYD_API boyd::GameState *Boyd_GameState();
