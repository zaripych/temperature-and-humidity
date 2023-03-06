#pragma once

#include <memory>
#include "action.hpp"
#include "actions_dispatcher_delegate.hpp"
#include "task_scheduler_delegate.hpp"
#include "actions_processor.hpp"

class actions_dispatcher : public actions_dispatcher_delegate
{
private:
    std::vector<std::shared_ptr<action>> actions;
    std::vector<actions_processor> processors;

protected:
    void emit_impl(const std::shared_ptr<action> &ptr) override
    {
        actions.push_back(ptr);
        if (actions.size() != 1)
        {
            return;
        }
        if (!scheduler)
        {
            return;
        }
        // after first action - schedule processing it
        scheduler->add_task([this]()
                            { process_events(); });
    }

    std::shared_ptr<cancel_timer_interface> set_emit_timeout_impl(
        double milliseconds,
        const std::shared_ptr<action> &ptr) override
    {
        if (!scheduler)
        {
            return nullptr;
        }
        return scheduler->set_timeout(milliseconds, [this, ptr]()
                                      { emit_impl(ptr); });
    };

    std::shared_ptr<cancel_timer_interface> set_emit_interval_impl(
        double milliseconds,
        const std::shared_ptr<action> &ptr) override
    {
        if (!scheduler)
        {
            return nullptr;
        }
        return scheduler->set_interval(milliseconds, [this, ptr]()
                                       { emit_impl(ptr); });
    };

    void process_events()
    {
        auto actions_copy = std::move(actions);
        actions.clear();
        process_events(actions_copy);
    }

    void process_events(const std::vector<std::shared_ptr<action>> &actions)
    {
        for (auto action_ptr : actions)
        {
            this->process_event(*action_ptr);
        }
    }

public:
    task_scheduler_delegate *scheduler;

    actions_dispatcher(task_scheduler_delegate *scheduler = nullptr) : scheduler(scheduler)
    {
    }

    void add_processor(actions_processor p)
    {
        processors.push_back(p);
    }

    void process_event(const action &action) override
    {
        for (auto processor : processors)
        {
            processor(action, *this);
        }
    }
};