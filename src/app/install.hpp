#pragma once

#include <Arduino.h>
#include <SparkFun_TMP117.h>
#include "framework/actions_dispatcher.hpp"
#include "framework/task_scheduler.hpp"

#include "features/temperature/tmp117_processor.hpp"
#include "features/temperature/ds18b20_processor.hpp"
#include "features/current/ina219_processor.hpp"
#include "features/power_management/processor.hpp"

#include "constants.hpp"
#include "action_logging.hpp"

#include "helpers/logger.h"
#include "helpers/rtc_data_store.hpp"

#include "features/display/u8g2_processor.hpp"
#include "reducer.hpp"
#include "u8g2_renderer.hpp"

void install_app(actions_dispatcher &dispatcher, task_scheduler &scheduler)
{
  static rtc_data_store store;

  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }

  Wire.begin();
  Wire.setTimeout(1000);
  Wire.setClock(400000);

  dispatcher.add_processor(create_u8g2_display_processor<ssd1306_display_id>(
      create_reducer(),
      create_u8g2_renderer(),
      scheduler,
      dispatcher));

  dispatcher.add_processor(action_logging_processor);

  dispatcher.add_processor(create_tmp117_temperature_processor<tmp117_sensor_id>(store));
  dispatcher.add_processor(create_ds18b20_temperature_processor<ds18b20_sensor_id>(store));
  dispatcher.add_processor(create_ina219_current_processor<ina219_sensor_id>());

  auto sleep_when_read_temperatures = [](const action &a) -> bool
  {
    static bool has_ds18b20_temperature = false;
    static bool has_tmp117_temperature = false;
    static bool idle = false;

    if (is_of_type<temperature_readout>(a))
    {
      if (a.domain == tmp117_sensor_id)
      {
        has_tmp117_temperature = true;
      }
      else if (a.domain == ds18b20_sensor_id)
      {
        has_ds18b20_temperature = true;
      }
    }
    if (auto idle_action = is_of_type<scheduler_idle>(a))
    {
      idle = idle_action.value().idle_for_microseconds > 5e6;
    }
    auto will_sleep = has_ds18b20_temperature && has_tmp117_temperature && idle;
    if (will_sleep)
    {
      store.save();
    }
    return will_sleep;
  };

  dispatcher.add_processor(
      create_power_management_processor(
          dispatcher,
          sleep_when_read_temperatures,
          {
            should_wakeup_on_touch : {
                {pin : T4, threshold : 30},
            }
          }));
}