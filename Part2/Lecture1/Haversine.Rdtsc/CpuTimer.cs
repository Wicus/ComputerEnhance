using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Haversine.Rdtsc;

public static class CpuTimer
{
    [DllImport("rdtsc.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern ulong ReadTSC();

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ulong ReadCpuTimer()
    {
        return ReadTSC();
    }

    public static ulong EstimateCpuFrequency()
    {
        var sw = Stopwatch.StartNew();
        var start = ReadTSC();

        Thread.Sleep(100);

        var end = ReadTSC();
        sw.Stop();

        var elapsed = end - start;
        var seconds = sw.Elapsed.TotalSeconds;

        return (ulong)(elapsed / seconds);
    }
}
