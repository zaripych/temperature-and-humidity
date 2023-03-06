#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <sstream>

#include "framework/actions_processor.hpp"
#include "features/temperature/readout.hpp"
#include "features/current/measurement.hpp"

#include "constants.hpp"
#include "helpers/render_string.hpp"

#include <map>

struct app_state
{
    optional<current_measurement> ina219;
    optional<temperature_readout> ds18b20;
    optional<temperature_readout> tmp117;

    int set_temp_status(const temperature_sensor_status &a)
    {
        if (a.is_connected)
        {
            return 0;
        }
        if (a.sensor_id == tmp117_sensor_id)
        {
            tmp117 = nullptr;
        }
        else if (a.sensor_id == ds18b20_sensor_id)
        {
            ds18b20 = nullptr;
        }
        return 1;
    }

    int set_temp(const temperature_readout &a)
    {
        if (a.sensor_id == tmp117_sensor_id)
        {
            tmp117 = a;
        }
        else if (a.sensor_id == ds18b20_sensor_id)
        {
            ds18b20 = a;
        }
        return 1;
    }
};

std::function<int(const action &, app_state &)> create_reducer()
{
    auto reduce_state = [=](const action &a, app_state &state) -> int
    {
        if (auto status = is_of_type<temperature_sensor_status>(a))
        {
            return state.set_temp_status(status.value());
        }
        if (auto readout = is_of_type<temperature_readout>(a))
        {
            return state.set_temp(readout.value());
        }
        if (auto measurement = is_of_type<current_measurement>(a))
        {
            state.ina219 = measurement.value();
            return 1;
        }
        if (auto status = is_of_type<current_sensor_status>(a))
        {
            if (!status.value().is_connected)
            {
                state.ina219 = nullptr;
                return 1;
            }
        }
        return 0;
    };

    return reduce_state;
};