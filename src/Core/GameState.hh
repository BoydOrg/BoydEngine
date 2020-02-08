#pragma once

#include "Platform.hh"
#include <array>
#include <atomic>
#include <entt/entt.hpp>

namespace boyd
{

/// The input state is being managed
struct BOYD_API InputState
{
    /// Axes:
    /// 0: Camera horiz, mapped from mouse cursor (-1 when turning left, +1 when turning right) as delta position between two frames
    /// 1: Camera vert, mapped from mouse cursor (-1 when turning down, +1 when turning up) as delta position between two frames
    /// 2: Exploration horiz, mapped from A (-1) and D(+1)
    /// 3: Exploration vert, mapped from W (-1) and D (+1)
    /// 4: Exploration2 horiz, mapped from LEFT (-1) and RIGHT (+1)
    /// 5: Exploration2 horiz, mapped from LEFT (-1) and RIGHT (+1)
    /// 6: Jump Trigger, mapped from Space (0: unpressed, 1: pressed)
    /// 7: Sprint Trigger, mapped from Shift (0: unpressed, 1: pressed)
    /// 8-33: If available, all the other alphabetic keys (0: unpressed, 1: pressed), else just a bunch of zeros
    /// 34-43: If available, all the other numeric keys (0: unpressed, 1: pressed).
    /// Supporting the gamepad would be nice but is currently not planned.
    ///
    /// The axis array is kept as an enum to make int castings easier for Lua
    enum Axis
    {
        AXIS_CAMERA_HORIZ = 0,
        AXIS_CAMERA_VERT = 1,
        AXIS_EXPLORATION_HORIZ = 2,
        AXIS_EXPLORATION_VERT = 3,
        AXIS_EXPLORATION2_HORIZ = 4,
        AXIS_EXPLORATION2_VERT = 5,
        AXIS_JUMP_TRIGGER = 6,
        AXIS_SPRINT_TRIGGER = 7,
        AXIS_OTHER_TRIGGER = 8
    };

    static constexpr int NUM_AXES = 35;
    float axes[NUM_AXES];
};

/// The whole game state, shared among modules.
struct BOYD_API GameState
{
    std::atomic<bool> running; ///< Set to false to request the loop to exit.
    entt::registry ecs;        ///< The EnTT ECS.
    InputState Input;          ///< The current input (at the last frame!)

    GameState()
        : running{true}, ecs{}
    {
    }
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
