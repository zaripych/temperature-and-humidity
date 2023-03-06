#pragma once

#include <string>
#include <sstream>

// reuse the buffer across all rendering functions
static std::stringstream buffer;

template <typename Fn>
std::string render_string(Fn fn)
{
    buffer.seekg(0);
    buffer.seekp(0);
    fn(buffer);
    buffer << std::ends;
    return buffer.str();
}
