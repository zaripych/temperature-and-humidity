#pragma once

#include "framework/action.hpp"

struct app_started : action
{
    constexpr static const char *action_type = "APP/STARTED";

    app_started() : action(action_type) {}
};