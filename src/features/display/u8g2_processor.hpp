#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "framework/actions_processor.hpp"
#include "framework/task_scheduler.hpp"
#include "framework/started.hpp"
#include "i2c/i2c_status.hpp"
#include "features/power_management/wakeup.hpp"
#include "features/power_management/prepare_to_deep_sleep.hpp"

#include <functional>

#include "display_status.hpp"

#include "helpers/logger.h"

template <const char *const display_id, typename State>
actions_processor create_u8g2_display_processor(
    // this function should mutate state according to the actions
    // passed down to it - if mutated it should return a value > 0
    const std::function<int(const action &, State &)> state_builder,
    // this function should take the state and display it
    const std::function<void(const State &, U8G2 &display)> renderer,
    task_scheduler &scheduler,
    actions_dispatcher_delegate &dispatcher,
    const int i2c_addr_display = 0x3C)
{
    static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
        U8G2_R0,
        U8X8_PIN_NONE,
        SCL,
        SDA);
    static State state;
    static bool display_connected = false;
    static bool needs_initialization = false;
    static bool woke_up = false;
    static int render_cycle = 0;
    static int render_target = 0;

    static auto &logger = get_logger(true);

    u8g2.setBusClock(400000);

    // refresh connection every time we are about to render something on the screen
    // this lives outside of action handling, but itself can be source of events;
    auto refresh_connection = [=, &dispatcher]()
    {
        const int i2c_display_status_timeout_millis = 10;

        if (!display_connected)
        {
            display_connected = i2c_status(
                i2c_addr_display,
                i2c_display_status_timeout_millis);
            if (display_connected)
            {
                needs_initialization = true;
            }
            if (display_connected)
            {
                dispatcher.emit(display_status(display_id, true));
            }
        }
        else
        {
            display_connected = i2c_status(
                i2c_addr_display,
                i2c_display_status_timeout_millis);
            if (!display_connected)
            {
                dispatcher.emit(display_status(display_id, false));
            }
        }
    };

    auto refresh_display_if_needed = [=]()
    {
        refresh_connection();
        if (!display_connected)
        {
            return;
        }
        if (render_cycle == render_target)
        {
            return;
        }
        if (needs_initialization)
        {
            needs_initialization = false;
            if (woke_up)
            {
                u8g2.initInterface();
                u8g2.sleepOff();
            }
            else
            {
                u8g2.begin();
                u8g2.sleepOff();
            }
        }
        // only rerender if connected to display and refresh is needed
        renderer(state, u8g2);
        render_cycle = render_target;
    };

    auto schedule_refresh_display = [=, &scheduler]()
    {
        if (render_cycle != render_target)
        {
            // already scheduled, this is the only way render_cycle != render_target
            return;
        }
        render_target = render_cycle + 1;
        scheduler.add_task(refresh_display_if_needed, task_type::immediate);
    };

    auto initialize_on_startup = [=](const action &a, actions_dispatcher_delegate &d)
    {
        if (is_of_type<app_started>(a))
        {
            schedule_refresh_display();
        }
        else if (is_of_type<wakeup>(a))
        {
            woke_up = true;
        }
    };

    auto on_display_status_change = [=](const action &a, actions_dispatcher_delegate &d)
    {
        if (auto status_action = is_of_type<display_status>(a))
        {
            if (status_action.value().is_connected)
            {
                schedule_refresh_display();
            }
        }
    };

    auto turn_off_display_before_deep_sleep = [=](const action &a, actions_dispatcher_delegate &d)
    {
        if (is_of_type<prepare_to_deep_sleep>(a))
        {
            u8g2.sleepOn();
        }
    };

    auto display_processor = [=](const action &a, actions_dispatcher_delegate &d)
    {
        initialize_on_startup(a, d);
        turn_off_display_before_deep_sleep(a, d);
        if (state_builder(a, state) > 0)
        {
            schedule_refresh_display();
        }
        if (a.domain != display_id)
        {
            return;
        }
        on_display_status_change(a, d);
    };

    return display_processor;
};