#include <stdint.h>

typedef uint64_t u64;

#ifdef _WIN32
#include <intrin.h>
#include <windows.h>
#define EXPORT __declspec(dllexport)
#else
#include <x86intrin.h>
#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT u64 ReadTimestampCounter(void)
{
    return __rdtsc();
}

#ifdef _WIN32
u64 ReadOsTimer()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

u64 ReadOsTimerFreq()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

u64 EstimateCpuTimerFreq()
{
    u64 cpuStart = ReadTimestampCounter();
    u64 osStart = ReadOsTimer();
    u64 osFreq = ReadOsTimerFreq();
    u64 osEnd = 0;
    u64 osElapsed = 0;
    u64 waitInMs = 100;
    u64 osWaitTimeInSeconds = osFreq * waitInMs / 1000;

    while (osElapsed < osWaitTimeInSeconds)
    {
        osEnd = ReadOsTimer();
        osElapsed = osEnd - osStart;
    }

    u64 cpuEnd = ReadTimestampCounter();
    u64 cpuElapsed = cpuEnd - cpuStart;

    u64 cpuFreq = 0;
    if (osElapsed)
    {
        cpuFreq = osFreq * cpuElapsed / osElapsed;
    }

    return cpuFreq;
}
#endif

#ifdef __cplusplus
}
#endif
