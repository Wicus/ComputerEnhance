// rdtsc.c — CPU timer utilities compiled into a shared library (librdtsc.so).
// Exposes ReadTimestampCounter and EstimateCpuTimerFreq for use via P/Invoke from C#.

#include <stdint.h>
#include <x86intrin.h>
#include <time.h>

typedef uint64_t u64;

// Marks a symbol as exported from the shared object.
// Without -fvisibility=hidden this is redundant, but makes intent explicit.
#define EXPORT __attribute__((visibility("default")))

// For Linux, we use clock_gettime with CLOCK_MONOTONIC_RAW, which has nanosecond resolution.
static const u64 unixOsClockFreq = 1000000000; // nanosecond resolution

static u64 ReadOsTimer(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (u64)ts.tv_sec * unixOsClockFreq + (u64)ts.tv_nsec;
}

static u64 ReadOsTimerFreq(void)
{
    return unixOsClockFreq;
}

EXPORT u64 ReadTimestampCounter(void)
{
    return __rdtsc();
}

// Estimates the CPU timer frequency (RDTSC ticks/second) by spinning against the OS clock for ~100ms.
EXPORT u64 EstimateCpuTimerFreq(void)
{
    u64 cpuStart = ReadTimestampCounter();
    u64 osStart = ReadOsTimer();
    u64 osFreq = ReadOsTimerFreq();
    u64 osWaitTicks = osFreq * 100 / 1000; // 100ms
    u64 osElapsed = 0;
    u64 osEnd = 0;

    while (osElapsed < osWaitTicks)
    {
        osEnd = ReadOsTimer();
        osElapsed = osEnd - osStart;
    }

    u64 cpuElapsed = ReadTimestampCounter() - cpuStart;

    if (osElapsed == 0)
    {
        return 0;
    }
    return osFreq * cpuElapsed / osElapsed;
}
