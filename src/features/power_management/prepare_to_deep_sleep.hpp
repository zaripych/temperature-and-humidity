#pragma once

#include "framework/action.hpp"

struct prepare_to_deep_sleep : action
{
    constexpr static const char *action_type = "POWER_MANAGEMENT/PREPARE_TO_DEEP_SLEEP";

    prepare_to_deep_sleep() : action(action_type) {}
};