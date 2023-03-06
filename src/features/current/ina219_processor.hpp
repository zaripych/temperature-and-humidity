#pragma once

#include <Arduino.h>
#include <INA219_WE.h>

#include "framework/actions_processor.hpp"
#include "framework/started.hpp"
#include "features/power_management/wakeup.hpp"
#include "i2c/i2c_status.hpp"

#include "measurement.hpp"
#include "refresh_required.hpp"
#include "sensor_status.hpp"

#include "helpers/sleep.h"

#include <functional>

template <const char *const sensor_id>
actions_processor create_ina219_current_processor(const int i2c_addr_sensor = 0x40)
{
    static INA219_WE ina219(i2c_addr_sensor);
    static bool connected = false;

    auto init_sensor = [=]()
    {
        ina219.setBusRange(INA219_BUS_RANGE::BRNG_16);
        ina219.setPGain(INA219_PGAIN::PG_40);
        ina219.setADCMode(INA219_ADC_MODE::SAMPLE_MODE_128);
        ina219.setMeasureMode(INA219_MEASURE_MODE::CONTINUOUS);
    };

    auto build_state = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (auto status_action = is_of_type<current_sensor_status>(action))
        {
            auto &status = status_action.value();
            if (!connected && status.is_connected)
            {
                init_sensor();
            }
            connected = status.is_connected;
        }
    };

    auto measure_when_required = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (is_of_type<current_refresh_required>(action))
        {
            if (!i2c_status(i2c_addr_sensor))
            {
                if (connected)
                {
                    dispatcher.emit(current_sensor_status(sensor_id, false));
                }
                return;
            }
            else
            {
                if (!connected)
                {
                    dispatcher.emit(current_sensor_status(sensor_id, true));
                }
            }
            if (connected)
            {
                // Read voltage and current from INA219.
                float shunt_voltage = ina219.getShuntVoltage_mV();
                float bus_voltage = ina219.getBusVoltage_V();
                float current_mA = ina219.getCurrent_mA();
                float power_mW = ina219.getBusPower();

                // Compute load voltage
                float load_voltage = bus_voltage + (shunt_voltage / 1000);

                dispatcher.emit(current_measurement(sensor_id,
                                                    load_voltage,
                                                    current_mA,
                                                    power_mW));
            }
        }
    };

    auto check_connected_on_startup = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (is_of_type<app_started>(action) || is_of_type<wakeup>(action))
        {
            if (i2c_status(i2c_addr_sensor) && ina219.init())
            {
                dispatcher.emit(current_sensor_status(sensor_id, true));
                dispatcher.emit(current_refresh_required(sensor_id));
            }
            else
            {
                dispatcher.emit(current_sensor_status(sensor_id, false));
            }
        }
    };

    auto processor = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        // process all incoming actions here
        check_connected_on_startup(action, dispatcher);
        if (action.domain != sensor_id)
        {
            return;
        }
        // process internal actions here
        build_state(action, dispatcher);
        measure_when_required(action, dispatcher);
    };

    return processor;
};