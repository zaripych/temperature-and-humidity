#include <esp_attr.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <esp_sleep.h>
#include <algorithm>

#include "time_of_day.h"

static RTC_DATA_ATTR int64_t before_sleep = 0;
static RTC_DATA_ATTR int time_to_sleep = 0;
static RTC_DATA_ATTR int64_t last_corrections[10];
static RTC_DATA_ATTR int current_correction = 0;
static RTC_DATA_ATTR int total_wakeups = 0;

int calculate_correction()
{
    auto correction = 0;
    auto length = std::min(total_wakeups, 10);
    for (auto i = 0; i < length; i += 1)
    {
        correction += last_corrections[i];
    }
    correction = correction / length;
    return correction;
}

void deep_sleep_auto_correct_wakeup_timer()
{
    auto now = time_of_day_microseconds();
    auto cause = esp_sleep_get_wakeup_cause();
    if (cause != ESP_SLEEP_WAKEUP_TIMER || time_to_sleep == 0)
    {
        return;
    }

    auto time_spent_sleeping_and_booting = now - before_sleep;
    auto overslept_by = time_spent_sleeping_and_booting - (time_to_sleep - current_correction);
    last_corrections[total_wakeups % 10] = overslept_by;
    total_wakeups += 1;

    current_correction = calculate_correction();
}

bool check_can_deep_sleep(uint64_t microseconds)
{
    return microseconds >= current_correction;
}

bool deep_sleep_with_auto_correction(uint64_t microseconds)
{
    if (microseconds < current_correction)
    {
        return false;
    }
    before_sleep = time_of_day_microseconds();
    time_to_sleep = microseconds;
    esp_sleep_enable_timer_wakeup(microseconds - current_correction);
    esp_deep_sleep_start();
    return true;
}
