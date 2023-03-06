#pragma once

#include <stdint.h>

void deep_sleep_auto_correct_wakeup_timer();

bool check_can_deep_sleep(uint64_t microseconds);

bool deep_sleep_with_auto_correction(uint64_t microseconds);
