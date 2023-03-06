#pragma once

#include "helpers/time_of_day.h"

class measure_time
{
private:
    int64_t timestamp;

public:
    measure_time() : timestamp(time_of_day_microseconds())
    {
    }

    measure_time(bool should_start) : timestamp(should_start ? time_of_day_microseconds() : 0)
    {
    }

    void start()
    {
        timestamp = time_of_day_microseconds();
    }

    void stop()
    {
        timestamp = 0;
    }

    bool is_started() const
    {
        return timestamp != 0;
    }

    int64_t started_at_us() const
    {
        return timestamp;
    }

    int64_t elapsed_us() const
    {
        if (timestamp == 0)
        {
            return 0;
        }
        return time_of_day_microseconds() - timestamp;
    }
};