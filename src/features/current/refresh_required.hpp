#pragma once

#include "framework/action.hpp"
#include "framework/sensor_action.hpp"
#include <sstream>

struct current_refresh_required : sensor_action
{
    constexpr static const char *action_type = "CURRENT/REFRESH_REQUIRED";

    current_refresh_required(const char *const sensor_id) : sensor_action(action_type, sensor_id)
    {
    }
};