#pragma once

enum optional_state_t
{
    empty = 0,
    has_value = 1,
};

template <typename Value>
class optional
{
private:
    static_assert(!std::is_reference<Value>::value, "Optional cannot store references");
    optional_state_t state;

    struct empty_t
    {
    };

    union
    {
        empty_t empty;
        Value payload;
    };

    void construct_payload(const Value &copy)
    {
        if (state == optional_state_t::has_value)
        {
            ::new (std::addressof(payload)) Value(copy);
        }
    }

    void destruct_payload()
    {
        if (state == optional_state_t::has_value)
        {
            payload.~Value();
        }
    }

public:
    optional() : state(optional_state_t::empty)
    {
    }

    optional(const optional<Value> &copy) : state(copy.state)
    {
        if (state == optional_state_t::has_value)
        {
            construct_payload(copy.payload);
        }
    }

    optional(const Value &value) : state(optional_state_t::has_value)
    {
        construct_payload(value);
    }

    optional(const Value &&value) : state(optional_state_t::has_value)
    {
        construct_payload(value);
    }

    ~optional()
    {
        destruct_payload();
    }

    operator bool() const
    {
        return state != optional_state_t::empty;
    }

    const Value &value() const
    {
        return payload;
    }

    optional<Value> &operator=(const Value &value)
    {
        destruct_payload();
        state = optional_state_t::has_value;
        construct_payload(value);
        return *this;
    }

    optional<Value> &operator=(nullptr_t value)
    {
        destruct_payload();
        state = optional_state_t::empty;
        return *this;
    }
};
