#pragma once

#include "framework/action.hpp"

enum wakeup_reasons
{
    rtc_io_interrupt,
    rtc_cntrl_interrupt,
    timer,
    touchpad,
    ultra_low_power_coprocessor,
};

struct wakeup : action
{
    constexpr static const char *action_type = "POWER_MANAGEMENT/WAKEUP";

    wakeup_reasons reason;

    uint16_t touch_pin;
    uint16_t touch_value;

    wakeup(wakeup_reasons reason) : action(action_type), reason(reason), touch_pin(0), touch_value(0) {}

    wakeup(
        wakeup_reasons reason,
        uint16_t touch_pin,
        uint16_t touch_value) : action(action_type),
                                reason(reason),
                                touch_pin(0),
                                touch_value(0) {}

    void format_to_stream(std::stringstream &stream) const override
    {
        action::format_to_stream(stream);
        switch (reason)
        {
        case wakeup_reasons::rtc_io_interrupt:
            stream << "(reason=rtc_io_interrupt)";
            break;
        case wakeup_reasons::rtc_cntrl_interrupt:
            stream << "(reason=rrc_cntrl_interrupt)";
            break;
        case wakeup_reasons::timer:
            stream << "(reason=timer)";
            break;
        case wakeup_reasons::touchpad:
            stream << "(reason=touchpad,pin=" << touch_pin << ",value=" << touch_value << ")";
            break;
        case wakeup_reasons::ultra_low_power_coprocessor:
            stream << "(reason=ultra_low_power_coprocessor)";
            break;
        default:
            break;
        }
    }
};