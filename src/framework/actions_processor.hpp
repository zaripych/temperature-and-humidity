#pragma once

#include "action.hpp"
#include "actions_dispatcher_delegate.hpp"

#include <functional>

typedef std::function<void(
    const action &action,
    actions_dispatcher_delegate &dispatcher)>
    actions_processor;
