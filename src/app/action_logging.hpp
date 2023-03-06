#pragma once

#include <helpers/logger.h>
#include <framework/action.hpp>
#include <framework/actions_dispatcher_delegate.hpp>
#include <framework/scheduler_idle.hpp>

void action_logging_processor(const action &a, actions_dispatcher_delegate &dispatcher)
{
    static auto idle_for = 0;
    static auto &logger = get_logger(true);

    if (auto idle = is_of_type<scheduler_idle>(a))
    {
        idle_for = idle.value().idle_for_microseconds;
        return;
    }
    else
    {
        if (idle_for > 0)
        {
            logger.printf("%s(%dms)\n", "SCHEDULER/IDLE", (int)(idle_for / 1e3));
        }
        idle_for = 0;
    }
    logger.printf("%s\n", a.description().c_str());
};