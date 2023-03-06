#pragma once

#include "framework/action.hpp"
#include "framework/sensor_action.hpp"
#include <helpers/render_string.hpp>

struct current_sensor_status : sensor_action
{
    constexpr static const char *action_type = "CURRENT/SENSOR_STATUS";

    bool is_connected;

    current_sensor_status(const char *const sensor_id,
                          bool is_connected) : sensor_action(action_type, sensor_id),
                                               is_connected(is_connected)
    {
    }

    void format_to_stream(std::stringstream &stream) const override
    {
        sensor_action::format_to_stream(stream);
        stream << "(connected=" << is_connected << ")";
    }
};