#pragma once

#include <list>
#include <memory>

#include "helpers/optional.hpp"
#include "helpers/time_of_day.h"
#include "helpers/measure_time.hpp"

#include "task.hpp"
#include "timer.hpp"
#include "scheduler_idle.hpp"
#include "task_scheduler_delegate.hpp"
#include "actions_dispatcher_delegate.hpp"

#include "helpers/logger.h"

class task_scheduler : public task_scheduler_delegate
{
private:
    typedef std::list<task_fn> tasks_t;
    // tasks to be run at current loop
    tasks_t immediate_tasks;
    tasks_t next_tasks;

    measure_time idle_time_counter;
    scheduler_idle idle_action;

public:
    actions_dispatcher_delegate *dispatcher = nullptr;

    task_scheduler() : idle_time_counter(false), idle_action(0)
    {
    }

    void run()
    {
        if (immediate_tasks.size() == 0 && next_tasks.size() == 0 && dispatcher)
        {
            if (!idle_time_counter.is_started())
            {
                idle_time_counter.start();
            }
            else
            {
                idle_action.idle_for_microseconds = idle_time_counter.elapsed_us();
            }
            dispatcher->process_event(idle_action);
        }
        else
        {
            idle_time_counter.stop();
        }
        // immediate tasks is mutated while we are iterating:
        while (immediate_tasks.size() > 0)
        {
            auto &task = immediate_tasks.front();
            task();
            immediate_tasks.pop_front();
        }
        immediate_tasks = std::move(next_tasks);
        next_tasks.clear();
    }

    void add_task(task_fn task,
                  task_type type = task_type::delayed) override
    {
        auto &push_to = type == task_type::immediate ? immediate_tasks : next_tasks;
        push_to.push_back(task);
    }

    timer_ref set_timeout(double milliseconds,
                          task_fn fn) override
    {
        return ::set_timeout(milliseconds, [this, fn]()
                             { add_task(fn); });
    };

    timer_ref set_interval(double milliseconds,
                           task_fn fn) override
    {
        return ::set_interval(milliseconds, [this, fn]()
                              { add_task(fn); });
    };
};