#include <cstdio>
#include <memory>

#include "BoydEngine.hh"
#include "Core/GameState.hh"
#include "Core/SceneManager.hh"
#include "Debug/Log.hh"
#include "Scripting/Lua.hh"

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

#include "raylib.h"

/// Raylib logging handler.
static void raylibLog(int msgType, const char *fmt, va_list args)
{
    static const LogLevel RAYLIB_TO_BOYD_LOGLEVEL[] = {
        LogLevel::Debug, // LOG_ALL
        LogLevel::Debug, // LOG_TRACE
        LogLevel::Debug, // LOG_DEBUG
        LogLevel::Info,  // LOG_INFO
        LogLevel::Warn,  // LOG_WARNING
        LogLevel::Error, // LOG_ERROR
        LogLevel::Crit,  // LOG_FATAL
    };
    static char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    buffer[255] = '\0';
    Log::instance().log(RAYLIB_TO_BOYD_LOGLEVEL[msgType], "<raylib>", 0, buffer);
}

int main(void)
{
    BOYD_LOG(Debug, "BoydEngine v{}.{}", BOYD_VERSION_MAJOR, BOYD_VERSION_MINOR);

    BOYD_MODULES_LIST();
    SetTraceLogCallback(raylibLog);

    // Make sure game state is inited
    (void)GameStateManager::Instance();
    (void)SceneManager::Instance();

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

#ifdef BOYD_HOT_RELOADING
    SetListener("lib/", 100);
#endif

    SceneManager::LoadScene("res/scene/1.scene");

    // Main game loop
    while(!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Technically this is a bad breach of any game loop rule, but Gfx makes drawing call
        // inside the updat method.
        BeginDrawing();
        UpdateModules();
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------

        //DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

#ifdef BOYD_HOT_RELOADING
    CloseListener();
#endif

    return 0;
}
