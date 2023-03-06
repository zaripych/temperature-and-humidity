#pragma once

#include "action.hpp"

struct sensor_action : action
{
    const char *const sensor_id;

    sensor_action(const char *const action_type,
                  const char *const sensor_id) : action(action_type, sensor_id),
                                                 sensor_id(sensor_id)
    {
    }
};