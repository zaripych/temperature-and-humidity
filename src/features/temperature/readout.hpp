#pragma once

#include "framework/action.hpp"
#include "framework/sensor_action.hpp"
#include <sstream>
#include <iomanip>

#include "helpers/sleep.h"

struct temperature_readout : sensor_action
{
    constexpr static const char *action_type = "TEMPERATURE/READOUT";

    double temp_c;
    int64_t timestamp;

    temperature_readout(const char *const sensor_id,
                        double temp_c) : sensor_action(action_type, sensor_id),
                                         temp_c(temp_c),
                                         timestamp(time_of_day_microseconds())
    {
    }

    void format_to_stream(std::stringstream &stream) const override
    {
        sensor_action::format_to_stream(stream);
        stream << std::setiosflags(std::ios_base::fixed)
               << std::setprecision(1)
               << "(" << temp_c << "°C)";
    }
};