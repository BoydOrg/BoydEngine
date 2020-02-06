#pragma once

// TODO: Find a way to automate this(?!)

#include "String.hh"
#include "Transform.hh"

/// Massive macro containing all types to register to the scripting system
/// (as `BOYD_REGISTER_TYPE(<typename>)` entries)
#define BOYD_REGISTER_ALLTYPES()              \
    BOYD_REGISTER_TYPE(boyd::comp::Transform) \
    BOYD_REGISTER_TYPE(boyd::comp::String)    \
// END
