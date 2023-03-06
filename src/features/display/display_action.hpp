#pragma once

#include "framework/action.hpp"
#include <sstream>

struct display_action : action
{
    const char *const display_id;

    display_action(const char *const action_type,
                   const char *const display_id) : action(action_type),
                                                   display_id(display_id)
    {
    }
};