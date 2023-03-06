#include <Arduino.h>
#include "sstream"
#include "helpers/time_of_day.h"
#include "logger.h"

struct no_op_logger : public Print
{
public:
    virtual size_t write(uint8_t byte) override
    {
        return 1;
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        return size;
    }
};

static RTC_DATA_ATTR uint64_t last_entry_timestamp = 0;

struct logger : public Print
{
private:
    bool ended_with_new_line = true;

public:
    virtual size_t write(uint8_t byte) override
    {
        return Serial.write(byte);
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        // if previous write ended with new line - prepend the timestamp
        if (ended_with_new_line)
        {
            if (last_entry_timestamp == 0)
            {
                last_entry_timestamp = time_of_day_microseconds();
                Serial.printf("[%'10.1fms] ", (float)0);
            }
            else
            {
                auto diff = time_of_day_microseconds() - last_entry_timestamp;
                last_entry_timestamp = time_of_day_microseconds();
                Serial.printf("[%'10.1fms] ", (float)diff / 1e3);
            }
        }
        ended_with_new_line =
            (size >= 1 && buffer[size - 1] == '\n') ||
            (size >= 2 && buffer[size - 2] == '\n' && buffer[size - 1] == 0);
        return Serial.write(buffer, size);
    }

    virtual int availableForWrite() override
    {
        return Serial.availableForWrite();
    }
};

Print &get_logger(bool enabled)
{
    static no_op_logger noop;
    static logger logger;
    if (enabled)
    {
        return logger;
    }
    else
    {
        return noop;
    }
}