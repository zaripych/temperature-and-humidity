#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "framework/actions_processor.hpp"
#include "framework/started.hpp"
#include "framework/sensor_action.hpp"
#include "features/power_management/wakeup.hpp"

#include "helpers/logger.h"

#include "sensor_status.hpp"
#include "readout.hpp"
#include "refresh_required.hpp"
#include "helpers/rtc_data_store.hpp"

#include <functional>

template <const char *const sensor_id, const int sensor_index = 0>
actions_processor create_ds18b20_temperature_processor(
    rtc_data_store &store,
    const int one_wire_pin = GPIO_ID_PIN(4))
{
    static OneWire oneWire(one_wire_pin);
    static DallasTemperature sensor(&oneWire);
    static bool sensor_connected = false;
    static auto &logger = get_logger(true);

    // 0 - initial
    // 1 - connected, saved
    // 2 - was never connected
    static int sensor_address_saved = 0;

    static uint8_t sensor_address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t *sensor_address_ptr = (uint8_t *)&sensor_address;

    store.bind(sensor_address_saved);
    store.bind(sensor_address);

    sensor.setWaitForConversion(false);

    auto initialize_sensor_address = [=]()
    {
        if (oneWire.reset() != 1)
        {
            logger.println("OneWire reset failed");
            return false;
        }
        if (sensor_address_saved == 1)
        {
            logger.println("Sensor address was previously saved, assuming connected");
            return true;
        }
        auto result = sensor.getAddress(sensor_address_ptr, sensor_index);
        if (!result)
        {
            // request twice if not connected 🤦‍♂️ ... works this way
            result = sensor.getAddress(sensor_address_ptr, sensor_index);
            if (!result)
            {
                logger.println("Failed to get sensor address");
            }
        }
        return result;
    };

    auto build_state = [=](
                           const action &action,
                           actions_dispatcher_delegate &dispatcher)
    {
        static timer_ref tm;
        if (auto status_action = is_of_type<temperature_sensor_status>(action))
        {
            sensor_connected = status_action.value().is_connected;
            if (sensor_connected)
            {
                sensor_address_saved = 1;
            }
            else
            {
                sensor_address_saved = 2;
            }
        }
    };

    auto check_sensor_connected_on_startup = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (is_of_type<app_started>(action) || is_of_type<wakeup>(action))
        {
            if (!initialize_sensor_address())
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, false));
            }
            else
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, true));
                dispatcher.emit(temperature_refresh_required(sensor_id));
            }
        }
    };

    auto connect_sensor_if_disconnected_when_readout_required = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (auto refresh_required = is_of_type<temperature_refresh_required>(action))
        {
            if (sensor_connected)
            {
                return;
            }
            if (initialize_sensor_address())
            {
                dispatcher.emit(temperature_sensor_status(sensor_id, true));
            }
        }
    };

    auto readout_temperature_when_refresh_required = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (auto refresh_required = is_of_type<temperature_refresh_required>(action))
        {
            if (sensor_address_saved != 1)
            {
                return;
            }
            if (!sensor.requestTemperaturesByAddress(sensor_address_ptr))
            {
                // request twice if not connected 🤦‍♂️ ... works this way
                if (!sensor.requestTemperaturesByAddress(sensor_address_ptr))
                {
                    dispatcher.emit(temperature_sensor_status(sensor_id, false));
                    return;
                };
            }

            auto result = sensor.getTempC(sensor_address_ptr);
            if (result != DEVICE_DISCONNECTED_C && result != 85)
            {
                dispatcher.emit(temperature_readout(sensor_id, result));
            }
        }
    };

    auto ds18b20_processor = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        check_sensor_connected_on_startup(action, dispatcher);
        if (action.domain != sensor_id)
        {
            return;
        }
        build_state(action, dispatcher);
        connect_sensor_if_disconnected_when_readout_required(action, dispatcher);
        readout_temperature_when_refresh_required(action, dispatcher);
    };

    return ds18b20_processor;
};