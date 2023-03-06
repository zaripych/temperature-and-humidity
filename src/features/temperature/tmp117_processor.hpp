#pragma once

#include <Arduino.h>
#include <SparkFun_TMP117.h>

#include "framework/actions_processor.hpp"
#include "framework/sensor_action.hpp"
#include "framework/started.hpp"
#include "features/power_management/wakeup.hpp"

#include "i2c/i2c_status.hpp"

#include "sensor_status.hpp"
#include "readout.hpp"
#include "refresh_required.hpp"

#include <functional>

#include "helpers/rtc_data_store.hpp"

template <const char *const sensor_id>
actions_processor create_tmp117_temperature_processor(
    rtc_data_store &store,
    const int i2c_addr_tmp_sensor = 0x48)
{
    static TMP117 sensor;
    static bool sensor_is_connected = false;

    store.bind(sensor_is_connected);

    auto build_state = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (auto sensor_status = is_of_type<temperature_sensor_status>(action))
        {
            sensor_is_connected = sensor_status.value().is_connected;
        }
    };

    auto readout_temperature_when_refresh_required = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        // if the sensor cannot provide us with the data immediately
        static timer_ref refresh_timer;

        if (auto refresh_required = is_of_type<temperature_refresh_required>(action))
        {
            if (!i2c_status(i2c_addr_tmp_sensor))
            {
                if (sensor_is_connected)
                {
                    dispatcher.emit(temperature_sensor_status(sensor_id, false));
                }
                return;
            }
            else
            {
                if (!sensor_is_connected)
                {
                    dispatcher.emit(temperature_sensor_status(sensor_id, true));
                }
            }
            if (sensor_is_connected && sensor.dataReady())
            {
                double temperatureCelsius = sensor.readTempC();
                dispatcher.emit(temperature_readout(sensor_id, temperatureCelsius));
            }
            else
            {
                refresh_timer = dispatcher.set_emit_timeout(1000, temperature_refresh_required(sensor_id));
            }
        }
    };

    auto check_sensor_connected_on_startup = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (is_of_type<app_started>(action))
        {
            if (i2c_status(i2c_addr_tmp_sensor) && sensor.begin(i2c_addr_tmp_sensor, Wire))
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, true));
                dispatcher.emit(temperature_refresh_required(sensor_id));
            }
            else
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, false));
            }
        }
        else if (is_of_type<wakeup>(action))
        {
            if (i2c_status(i2c_addr_tmp_sensor) && sensor.begin(i2c_addr_tmp_sensor, Wire))
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, true));
                dispatcher.emit(temperature_refresh_required(sensor_id));
            }
            else
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, false));
            }
        }
    };

    auto tmp17_temperature_processor = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        check_sensor_connected_on_startup(action, dispatcher);
        if (action.domain != sensor_id)
        {
            return;
        }
        build_state(action, dispatcher);
        readout_temperature_when_refresh_required(action, dispatcher);
    };

    return tmp17_temperature_processor;
};