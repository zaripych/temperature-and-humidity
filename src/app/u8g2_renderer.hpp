#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <sstream>

#include "framework/actions_processor.hpp"
#include "features/temperature/readout.hpp"
#include "features/current/measurement.hpp"

#include "constants.hpp"
#include "helpers/render_string.hpp"
#include "helpers/logger.h"

template <typename FieldType>
std::string render_field(const char *name, const FieldType &value, const std::string &unit = "")
{
    return render_string([=](std::stringstream &stream)
                         { stream
                               << name
                               << ": "
                               << std::setw(15 - stream.tellp())
                               << std::setiosflags(std::ios_base::fixed)
                               << std::setprecision(1)
                               << value
                               << unit; });
}

std::function<void(const app_state &, U8G2 &display)> create_u8g2_renderer()
{
    static auto &logger = get_logger(true);

    auto render_state = [=](const app_state &state, U8G2 &display)
    {
        display.clearBuffer();
        display.setFont(u8g2_font_t0_14_tr);

        auto line_height = 12;
        auto y = line_height;

        std::string tmp117_text = state.tmp117
                                      ? render_field("tmp117", state.tmp117.value().temp_c, "C")
                                      : render_field("tmp117", "[x]");
        display.drawUTF8(0, y, tmp117_text.c_str());
        y += line_height;

        std::string ds18b20_text = state.ds18b20
                                       ? render_field("ds18b20", state.ds18b20.value().temp_c, "C")
                                       : render_field("ds18b20", "[x]");
        display.drawUTF8(0, y, ds18b20_text.c_str());
        y += line_height;

        if (state.ina219)
        {
            auto measurements = state.ina219.value();
            std::string voltage = render_field("voltage", measurements.voltage_volts, "V");
            display.drawUTF8(0, y, voltage.c_str());
            y += line_height;
            std::string current = render_field("current", measurements.current_mA, "mA");
            display.drawUTF8(0, y, current.c_str());
            y += line_height;
            std::string power = render_field("power", measurements.power_mW, "mW");
            display.drawUTF8(0, y, power.c_str());
            y += line_height;
        }
        else
        {
            display.drawUTF8(0, y, "[x]");
            y += line_height;
        }

        display.sendBuffer();
    };

    return render_state;
};