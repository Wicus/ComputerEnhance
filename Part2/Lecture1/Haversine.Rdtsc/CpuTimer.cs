using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Haversine.Rdtsc;

public static class CpuTimer
{
    // Load logical name "rdtsc" so the runtime resolves:
    //   Windows: rdtsc.dll
    //   Linux:   librdtsc.so
    //   macOS:   librdtsc.dylib (if provided)
    [DllImport("rdtsc", CallingConvention = CallingConvention.Cdecl)]
    private static extern ulong ReadTSC();

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong ReadCpuTimer()
    {
        return ReadTSC();
    }

    public static ulong EstimateCpuFrequency()
    {
        const double targetSeconds = 1.00;
        var stopwatchFrequency = Stopwatch.Frequency;
        var sampleTicks = (long)(targetSeconds * stopwatchFrequency);

        var originalPriority = Thread.CurrentThread.Priority;
        Thread.CurrentThread.Priority = ThreadPriority.Highest;

        try
        {
            var start = Sample();
            var spin = new SpinWait();

            while (Stopwatch.GetTimestamp() - start.StopwatchTicks < sampleTicks)
            {
                spin.SpinOnce();
            }

            var end = Sample();

            var elapsedTsc = end.Tsc - start.Tsc;
            var elapsedStopwatchTicks = end.StopwatchTicks - start.StopwatchTicks;

            if (elapsedStopwatchTicks <= 0)
            {
                return 0;
            }

            var frequency = (double)elapsedTsc * stopwatchFrequency / elapsedStopwatchTicks;
            return (ulong)Math.Round(frequency);
        }
        finally
        {
            Thread.CurrentThread.Priority = originalPriority;
        }
    }

    private static (ulong Tsc, long StopwatchTicks) Sample()
    {
        var maxSkew = Math.Max(Stopwatch.Frequency / 10_000, 1); // ~100 µs guard

        while (true)
        {
            var before = Stopwatch.GetTimestamp();
            var tsc = ReadTSC();
            var after = Stopwatch.GetTimestamp();

            if (after - before <= maxSkew)
            {
                var midpoint = before + ((after - before) >> 1);
                return (tsc, midpoint);
            }
        }
    }
}
