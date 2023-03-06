#pragma once

#include <Arduino.h>

#include "framework/actions_processor.hpp"
#include "framework/scheduler_idle.hpp"
#include "framework/task_scheduler.hpp"

#include "helpers/sleep.h"
#include "helpers/logger.h"

#include "framework/started.hpp"
#include "wakeup.hpp"
#include "prepare_to_deep_sleep.hpp"

void emit_startup_or_wakeup_once(actions_dispatcher_delegate &dispatcher)
{
    static auto emitted = false;
    if (!emitted)
    {
        emitted = true;
        esp_sleep_wakeup_cause_t wakeup_reason;
        wakeup_reason = esp_sleep_get_wakeup_cause();
        touch_pad_t pin = esp_sleep_get_touchpad_wakeup_status();
        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
            dispatcher.emit(wakeup(wakeup_reasons::rtc_io_interrupt));
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            dispatcher.emit(wakeup(wakeup_reasons::rtc_cntrl_interrupt));
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            dispatcher.emit(wakeup(wakeup_reasons::timer));
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            dispatcher.emit(wakeup(wakeup_reasons::touchpad, pin, 0));
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            dispatcher.emit(wakeup(wakeup_reasons::ultra_low_power_coprocessor));
            break;
        default:
            dispatcher.emit(app_started());
            break;
        }
    }
}

struct wakeup_pin_config
{
    uint8_t pin;
    uint16_t threshold;
};

struct power_management_config
{
    std::vector<wakeup_pin_config> should_wakeup_on_touch = {};
};

static void noop() {}

actions_processor create_power_management_processor(
    actions_dispatcher_delegate &dispatcher,
    std::function<bool(const action &)> should_sleep,
    power_management_config config = {})
{
    static bool should_sleep_state = false;
    static auto &logger = get_logger(true);

    static prepare_to_deep_sleep prepare;
    // when emitting prepare_to_deep_sleep do not
    // go into infinite loop of handling our own
    // event
    static bool prevent_recursive_loop = false;

    emit_startup_or_wakeup_once(dispatcher);

    auto reduce_state = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        should_sleep_state = should_sleep(action);
    };

    auto determine_sleep_time = []()
    {
        auto now = esp_timer_get_time();
        auto next = esp_timer_get_next_alarm();
        if ((int)next == -1)
        {
            int64_t default_sleep_time = 60e6;
            return default_sleep_time;
        }
        return next - now;
    };

    auto emit_prepare_to_deep_sleep = [&]()
    {
        prevent_recursive_loop = true;
        dispatcher.process_event(prepare);
        prevent_recursive_loop = false;
    };

    auto deep_sleep_when_required = [=]()
    {
        if (!should_sleep_state)
        {
            return;
        }
        if (config.should_wakeup_on_touch.size() > 0)
        {
            for (auto conf : config.should_wakeup_on_touch)
            {
                if (touchRead(conf.pin) < conf.threshold)
                {
                    return;
                }
            }
        }
        emit_prepare_to_deep_sleep();

        auto sleep_for = determine_sleep_time();
        if (!check_can_deep_sleep(sleep_for))
        {
            logger.printf("Cannot sleep for %d (it will take longer to wakeup)\n", (int)sleep_for);
            return;
        }

        if (sleep_for > 0)
        {
            logger.printf("Will sleep for %d\n", (int)sleep_for);
            if (config.should_wakeup_on_touch.size() > 0)
            {
                for (auto conf : config.should_wakeup_on_touch)
                {
                    touchAttachInterrupt(conf.pin, noop, conf.threshold);
                }
                esp_sleep_enable_touchpad_wakeup();
            }
            deep_sleep_with_auto_correction(sleep_for);
        }
    };

    auto processor = [=](const action &action, actions_dispatcher_delegate &dispatcher)
    {
        if (prevent_recursive_loop)
        {
            return;
        }
        reduce_state(action, dispatcher);
        deep_sleep_when_required();
    };

    return processor;
}