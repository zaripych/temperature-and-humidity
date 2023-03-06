#pragma once

#include <memory>
#include "task.hpp"

enum task_type
{
    delayed = 0,
    immediate = 1
};

struct task_scheduler_delegate
{
public:
    virtual void add_task(task_fn fn,
                          task_type type = task_type::delayed) = 0;

    virtual timer_ref set_timeout(double milliseconds,
                                  task_fn fn) = 0;

    virtual timer_ref set_interval(double milliseconds,
                                   task_fn fn) = 0;
};