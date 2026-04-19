using System.Runtime.InteropServices;

namespace Haversine.CpuTimer;

public static class CpuTimer
{
    // Returns the current CPU timestamp counter (RDTSC).
    // Ticks have no fixed unit — divide by EstimateFrequency() to convert to seconds.
    [DllImport("rdtsc", CallingConvention = CallingConvention.Cdecl, EntryPoint = "ReadTimestampCounter")]
    private static extern ulong _ReadTimestampCounter();
    public static ulong ReadTimestampCounter()
    {
        return _ReadTimestampCounter();
    }

    // Estimates the CPU timer frequency (ticks/second) by spinning for ~100ms.
    // Call once at startup and cache the result — do not call in a hot path.
    [DllImport("rdtsc", CallingConvention = CallingConvention.Cdecl, EntryPoint = "EstimateCpuTimerFreq")]
    public static extern ulong EstimateFrequency();
}
