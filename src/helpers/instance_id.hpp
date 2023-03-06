#pragma once

int next_id()
{
    static int id = 0;
    return id += 1;
}