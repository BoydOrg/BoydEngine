#include <cstdio>

#include "Debug/Log.hh"
#include "Modules/Loader.hh"
#include "BoydEngine.hh"

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

    // Main game loop
    while(!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateModules();
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    CloseListener();

    return 0;
}
