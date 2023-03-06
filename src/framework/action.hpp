#pragma once

#include <Arduino.h>

#include <typeinfo>
#include <cstring>
#include <string>
#include "helpers/optional.hpp"

#include "helpers/render_string.hpp"

constexpr const char ANY_ACTION[] = "";

// Action without template parameter for basic polymorphism of action pointer
struct action
{
    const char *const type;
    const char *const domain;

    action(const char *const type) : type(type), domain(nullptr){};
    action(const char *const type, const char *const domain) : type(type),
                                                               domain(domain){};

    action(const action &copy) = default;

    template <typename T>
    explicit operator const T() const;

    virtual const std::string description() const
    {
        auto format_lambda = [&](std::stringstream &stream) -> void
        {
            this->format_to_stream(stream);
        };
        return render_string(format_lambda);
    }

    virtual void format_to_stream(std::stringstream &stream) const
    {
        stream << type;
        if (domain)
        {
            stream << "::" << domain;
        }
    }
};

bool is_of_type(const action &a, const char *const type)
{
    // Figure out if we can just do a.type == type
    return a.type == type;
}

template <typename A>
const optional<A> is_of_type(const action &a)
{
    static_assert(std::is_base_of<action, A>(),
                  "A should be derived from `action`");
    return is_of_type(a, A::action_type) ? optional<A>((const A &)a) : optional<A>();
}

template <typename Action>
bool when_of_type(const action &a, void (*handler)(const Action &result))
{
    if (is_of_type<Action>(a))
    {
        handler(reinterpret_cast<Action>(a));
        return true;
    }
    return false;
}

template <typename T>
action::operator const T() const
{
    if (!is_of_type<T>(*this))
    {
        throw std::bad_cast();
    }
    return *reinterpret_cast<const T *>(this);
}
