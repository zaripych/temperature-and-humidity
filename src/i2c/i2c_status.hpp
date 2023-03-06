#pragma once

#include <Arduino.h>

bool i2c_status(int addr, int timeoutMillis = 0)
{
    auto previous = timeoutMillis != 0 ? Wire.getTimeout() : timeoutMillis;
    if (timeoutMillis != 0)
    {
        Wire.setTimeout(timeoutMillis);
    }
    Wire.beginTransmission(addr);
    auto code = Wire.endTransmission();
    if (code != 0)
    {
        // logger.printf("i2c: 0x%x %d - %s\n", addr, code, Wire.getErrorText(code));
        // Wire.clearWriteError();
        // had_i2c_issues = true;
    }
    if (timeoutMillis != 0)
    {
        Wire.setTimeout(previous);
    }
    return code == 0;
}