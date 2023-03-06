#pragma once

#include <memory>
#include "action.hpp"
#include "timer.hpp"

class actions_dispatcher_delegate
{
protected:
    virtual void emit_impl(const std::shared_ptr<action> &ptr) = 0;

    virtual std::shared_ptr<cancel_timer_interface> set_emit_timeout_impl(
        double milliseconds,
        const std::shared_ptr<action> &ptr) = 0;

    virtual std::shared_ptr<cancel_timer_interface> set_emit_interval_impl(
        double milliseconds,
        const std::shared_ptr<action> &ptr) = 0;

public:
    actions_dispatcher_delegate()
    {
    }

    template <typename Action>
    void emit(const Action &a)
    {
        this->emit_impl(std::shared_ptr<action>(new Action(a)));
    }

    template <typename Action>
    std::shared_ptr<cancel_timer_interface> set_emit_timeout(
        double milliseconds,
        const Action &a)
    {
        return this->set_emit_timeout_impl(milliseconds, std::shared_ptr<action>(new Action(a)));
    }

    template <typename Action>
    std::shared_ptr<cancel_timer_interface> set_emit_interval(
        double milliseconds,
        const Action &a)
    {
        return this->set_emit_interval_impl(milliseconds, std::shared_ptr<action>(new Action(a)));
    }

    virtual void process_event(const action &actions) = 0;
};