#pragma once

#include "framework/action.hpp"
#include <sstream>
#include "display_action.hpp"
#include "helpers/render_string.hpp"

struct display_status : display_action
{
    constexpr static const char *action_type = "DISPLAY/STATUS";

    bool is_connected;

    display_status(const char *const display_id,
                   bool is_connected) : display_action(action_type, display_id),
                                        is_connected(is_connected)
    {
    }

    void format_to_stream(std::stringstream &stream) const override
    {
        display_action::format_to_stream(stream);
        stream << "(connected=" << is_connected << ")";
    }
};