#pragma once

#include "action.hpp"

struct scheduler_idle : action
{
    constexpr static const char *action_type = "SCHEDULER/IDLE";

    int64_t idle_for_microseconds;

    scheduler_idle(int64_t idle_for_microseconds) : action(action_type),
                                                    idle_for_microseconds(idle_for_microseconds) {}
};
