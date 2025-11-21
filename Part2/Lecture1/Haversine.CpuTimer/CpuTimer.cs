using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Haversine.CpuTimer;

public static class CpuTimer
{
    [DllImport("rdtsc", CallingConvention = CallingConvention.Cdecl, EntryPoint = "ReadTimestampCounter")]
    private static extern ulong _ReadTimestampCounter();

    public static ulong ReadTimestampCounter()
    {
        return _ReadTimestampCounter();
    }

    public static ulong EstimateFrequency()
    {
        var cpuStart = _ReadTimestampCounter(); // __rdtsc
        var osStart = (ulong)Stopwatch.GetTimestamp(); // QueryPerformanceCounter
        var osFreq = (ulong)Stopwatch.Frequency; // QueryPerformanceFrequency | 10_000_000 on linux
        var waitTimeInMs = 100ul;
        var osWaitTicks = osFreq * waitTimeInMs / 1000;
        var osElapsedTicks = 0ul;

        while (osElapsedTicks < osWaitTicks)
        {
            var osEnd = (ulong)Stopwatch.GetTimestamp();
            osElapsedTicks = osEnd - osStart;
        }

        var cpuEnd = _ReadTimestampCounter();
        var cpuElapsedTicks = cpuEnd - cpuStart;

        if (osElapsedTicks == 0)
        {
            return 0;
        }

        var cpuFrequency = cpuElapsedTicks * osFreq / osElapsedTicks;
        return cpuFrequency;
    }
}
