#include "../../Core/GameState.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include "Gfx.hh"
#include <algorithm>

namespace boyd
{
void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void GLFWMouseCallback(GLFWwindow *window, double xpos, double ypos);

void InitInput(BoydGfxState *state)
{
    glfwSetKeyCallback(state->window, GLFWKeyCallback);
    glfwSetCursorPosCallback(state->window, GLFWMouseCallback);
#ifndef DEBUG
    glfwSetInputMode(state->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
}

/// Placeholder for something else later on
void UpdateInput(BoydGfxState *state)
{
    glfwPollEvents();
}

void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    InputState &input = Boyd_GameState()->Input;

    int coeff = (action == GLFW_PRESS);

    switch(key)
    {
    //  Map WASD keys to exploration axes
    case GLFW_KEY_W:
        input.axes[input.AXIS_EXPLORATION_VERT] = coeff;
        break;
    case GLFW_KEY_S:
        input.axes[input.AXIS_EXPLORATION_VERT] = -coeff;
        break;
    case GLFW_KEY_A:
        input.axes[input.AXIS_EXPLORATION_HORIZ] = -coeff;
        break;
    case GLFW_KEY_D:
        input.axes[input.AXIS_EXPLORATION_HORIZ] = coeff;
        break;

    // ... special modifiers
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        input.axes[input.AXIS_SPRINT_TRIGGER] = coeff;
        break;
    case GLFW_KEY_SPACE:
        input.axes[input.AXIS_JUMP_TRIGGER] = coeff;
        break;
    }

    // ... numeric characters
    if(key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
    {
        input.axes[input.AXIS_NUMERIC_TRIGGER + (key - GLFW_KEY_0)] = coeff;
    }

    // ... and alphabetic characters...
    else if(key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
    {
        input.axes[input.AXIS_ALPHABETIC_TRIGGER + (key - GLFW_KEY_A)] = coeff;
    }

    if(input.axes[input.AXIS_ALPHABETIC_TRIGGER + ('U' - 'A')])
    {
        BOYD_LOG(Info, "Ungrabbing...");
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void GLFWMouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    InputState &input = Boyd_GameState()->Input;

    float deltaX = xpos - input.mousePosition.xpos;
    float deltaY = ypos - input.mousePosition.ypos;

    input.mousePosition.xpos = xpos;
    input.mousePosition.ypos = ypos;

    int width, height;

    glfwGetWindowSize(window, &width, &height);

    /// TODO: consider multiplying these by a scaling factor (aka "sensitivity")
    input.axes[input.AXIS_CAMERA_HORIZ] = deltaX / width;
    input.axes[input.AXIS_CAMERA_VERT] = deltaY / height;

    //BOYD_LOG(Info, "Mouse axes: {} {}", input.axes[input.AXIS_CAMERA_HORIZ], input.axes[input.AXIS_CAMERA_VERT]);
}

} // namespace boyd