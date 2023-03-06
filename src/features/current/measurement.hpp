#pragma once

#include "framework/action.hpp"
#include "framework/sensor_action.hpp"
#include <sstream>
#include <iomanip>

#include "helpers/sleep.h"
#include "helpers/render_string.hpp"

struct current_measurement : sensor_action
{
    constexpr static const char *action_type = "CURRENT/MEASUREMENT";

    double voltage_volts;
    double current_mA;
    double power_mW;
    int64_t timestamp;

    current_measurement(const char *const sensor_id,
                        double voltage_volts,
                        double current_mA,
                        double power_mW) : sensor_action(action_type, sensor_id),
                                           voltage_volts(voltage_volts),
                                           current_mA(current_mA),
                                           power_mW(power_mW),
                                           timestamp(time_of_day_microseconds())
    {
    }

    void format_to_stream(std::stringstream &stream) const override
    {
        sensor_action::format_to_stream(stream);
        stream << std::setiosflags(std::ios_base::fixed)
               << std::setprecision(1)
               << "(\n"
               << "  "
               << voltage_volts << "v, "
               << "  "
               << current_mA << "mA, "
               << "  "
               << power_mW << "mW "
               << "\n)";
    }
};