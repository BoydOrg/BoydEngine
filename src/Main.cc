#include <cstdio>
#include <memory>

#include "BoydEngine.hh"
#include "Core/GameState.hh"
#include "Core/SceneManager.hh"
#include "Debug/Log.hh"

// clang-format off
#define BOYD_MODULE(name, priority) \
    extern "C" { \
        BOYD_API void *BoydInit_##name(void); \
        BOYD_API void  BoydUpdate_##name(void *); \
        BOYD_API void  BoydHalt_##name(void *); \
    }
// clang-format on
BOYD_MODULES_LIST()

#undef BOYD_MODULE

#include "Modules/Loader.hh" // this redefines BOYD_MODULE

using namespace boyd;

int main(void)
{
    BOYD_LOG(Debug, "BoydEngine v{}.{}", BOYD_VERSION_MAJOR, BOYD_VERSION_MINOR);

    BOYD_MODULES_LIST();

    // Make sure game state is inited
    (void)GameStateManager::Instance();
    (void)SceneManager::Instance();

#ifdef BOYD_HOT_RELOADING
    SetListener("lib/", 100);
#endif

    SceneManager::LoadScene("res/scene/1.scene");

    // Main game loop
    while(true) // FIXME: readd quitting...
    {
        // NOTE: Gfx is treated as a normal system (it makes drawcalls inside of its update() method).
        UpdateModules();
    }

#ifdef BOYD_HOT_RELOADING
    CloseListener();

    // HACK(PAOLO) If hot reloading is enabled, teardown should follow this order:
    // 1) Halt each of the modules - but do NOT `dlclose()` their libraries
    // 2) Delete the GameState
    // 3) `dlclose()` all modules
    // Failure to follow this sequence can currently result in crashes-on-exit due to `dlclose()`d libraries!
    for(auto &module : modules)
    {
        module.HaltFunc(module.data);
        module.HaltFunc = nullptr;
    }
    GameStateManager::Instance().~GameStateManager();
#endif
    // Force all modules to be unloaded before the GameState singleton is destroyed.
    // Important to prevent crashes on exit!
    modules.clear();

    return 0;
}
