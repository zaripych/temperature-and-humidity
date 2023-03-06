#pragma once

#include <Arduino.h>
#include <sstream>
#include <vector>
#include <functional>

#include "helpers/logger.h"

#if SERIALIZER
#include "serializer.h"

static RTC_DATA_ATTR bool saved_once = false;
static RTC_DATA_ATTR unsigned char common_space[256] = {0};

#endif

// This data store is not a silver bullet and should only be used
// when a global static variable, like above - cannot be declared.
//
// This could be if a static variable is scoped to function, or when that function
// is a template function, which is where RTC_DATA_ATTR wouldn't work unfortunately.
struct rtc_data_store
{
private:
#if SERIALIZER
    zpp::serializer::memory_view_output_archive out;
    zpp::serializer::memory_view_input_archive in;
    std::vector<std::function<void(zpp::serializer::memory_view_output_archive &)>> savers;
#endif

public:
#if SERIALIZER
    rtc_data_store() : out((unsigned char *)&common_space, sizeof(common_space)),
                       in((unsigned char *)&common_space, sizeof(common_space))
    {
    }
#endif

    // Bind a static variable to RTC memory block
    //
    // - if the device just started for the first time, this will remember the reference
    //   to the static variable and save the static variable into RTC memory block
    //   in binary form, which should be called before going to deep sleep
    //
    // - if the device has been awoken after deep sleep, this will deserialize data
    //   from RTC memory block into the variable specified, so that last value from
    //   previous run is available to current run
    //
    // NOTE: Calls to bind should be unconditional, preferrably next to the static
    // variable declaration - otherwise the shape of the data serialized might be
    // in incorrect state depending on condition;
    // NOTE: Calls to save should be unconditional as well, otherwise
    // there is risk of using outdated data
    template <typename Primitive>
    void bind(Primitive &value)
    {
#if SERIALIZER
        if (saved_once)
        {
            in(value);
        }
        savers.push_back([&value](zpp::serializer::memory_view_output_archive &out) -> void
                         { out(value); });
#endif
    }

    // Save current values for all bound variables into RTC memory block
    void save()
    {
        out.reset(0);
#if SERIALIZER
        try
        {
            for (auto saver : savers)
            {
                saver(out);
            }
            saved_once = true;
        }
        catch (std::exception &err)
        {
            get_logger(true).printf("Size: %d", (int)sizeof(common_space));
            get_logger(true).println(err.what());
        }
#endif
    }
};