#include <stdint.h>
#include <sys/time.h>

uint64_t time_of_day_microseconds()
{
    struct timeval timeval_now;
    gettimeofday(&timeval_now, NULL);
    return (int64_t)timeval_now.tv_sec * 1e6L + (int64_t)timeval_now.tv_usec;
}
