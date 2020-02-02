#pragma once
#include "Platform.hh"
#include <entt/entt.hpp>
#include <filesystem>
#include <string>
#include <vector>

/// This is just a stub that will likely be replaced with a more decent level loader

namespace boyd
{

struct BOYD_API SceneManagerState
{
    std::vector<entt::entity> entities;
};

/// A singleton used to manage a `GameState`.
class BOYD_API SceneManager
{
private:
    SceneManager()
    {
        state = new SceneManagerState();
    }

public:
    static SceneManager &Instance()
    {
        static SceneManager inst;
        return inst;
    }

    /// Load a scene with a give name
    /// `scene` - A path to take the scene description from
    static void LoadScene(const std::filesystem::path &scene);

    ~SceneManager()
    {
        delete state;
        state = nullptr;
    }

    inline operator SceneManagerState *()
    {
        return state;
    }

private:
    SceneManagerState *state;
};

} // namespace boyd

// Make the scene loader accessible from the modules.
extern "C" BOYD_API boyd::SceneManagerState *Boyd_SceneManager();