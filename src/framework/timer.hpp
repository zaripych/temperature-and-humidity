#pragma once

#include <esp_timer.h>
#include <string.h>
#include <memory>
#include <vector>
#include <algorithm>
#include "task.hpp"

static std::vector<void *> alive_timers;
static void timer_callback(void *arg);

struct cancel_timer_interface
{
    void cancel();
};

class timer : public cancel_timer_interface
{
    friend void timer_callback(void *arg);

private:
    esp_timer_handle_t handle;
    esp_timer_create_args_t args;
    task_fn task;

    void execute_from_timer()
    {
        task();
    }

public:
    timer(task_fn task) : handle(0), task(task)
    {
        memset(&args, 0, sizeof(esp_timer_create_args_t));
        args.callback = timer_callback;
        args.arg = this;
        esp_timer_create(&args, &handle);
        alive_timers.push_back(this);
    }

    void start_once(double milliseconds)
    {
        esp_timer_start_once(handle, milliseconds * 1e3);
    }

    void start_periodic(double milliseconds)
    {
        esp_timer_start_periodic(handle, milliseconds * 1e3);
    }

    void cancel()
    {
        esp_timer_stop(handle);
    }

    ~timer()
    {
        if (handle == 0)
        {
            return;
        }
        esp_timer_stop(handle);
        esp_timer_delete(handle);
        handle = 0;
        alive_timers.erase(std::find(alive_timers.begin(), alive_timers.end(), this));
    }
};

static void timer_callback(void *arg)
{
    auto timer_ptr = reinterpret_cast<timer *>(arg);
    if (timer_ptr->handle != 0 &&
        std::find(alive_timers.begin(), alive_timers.end(), timer_ptr) != alive_timers.end())
    {
        // is it possible that the timer object is destructed
        // just before the callback call? - unknown
        timer_ptr->execute_from_timer();
    }
};

typedef std::shared_ptr<cancel_timer_interface> timer_ref;

timer_ref set_timeout(double milliseconds,
                      task_fn fn)
{
    auto once_timer = std::shared_ptr<timer>(new timer(fn));
    once_timer->start_once(milliseconds);
    return once_timer;
}

timer_ref set_interval(double milliseconds,
                       task_fn fn)
{
    auto once_timer = std::shared_ptr<timer>(new timer(fn));
    once_timer->start_periodic(milliseconds);
    return once_timer;
}
