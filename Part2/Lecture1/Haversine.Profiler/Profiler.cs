using System.Diagnostics;

namespace Haversine.Profiler;

public interface IProfiler
{
    IDisposable BeginZone(string name);
}

public class Profiler : IProfiler
{
    private readonly Dictionary<string, ProfileZone> _zones = [];
    private readonly Stopwatch _globalTimer = Stopwatch.StartNew();
    private readonly ulong _cpuFrequency;

    public Profiler()
    {
        _cpuFrequency = EstimateCpuFrequency();
    }

    public IDisposable BeginZone(string name)
    {
        if (!_zones.TryGetValue(name, out var zone))
        {
            zone = new ProfileZone(name);
            _zones[name] = zone;
        }
        zone.Begin();
        return zone;
    }

    public void PrintResults(long totalBytes)
    {
        var totalElapsed = _globalTimer.Elapsed.TotalSeconds;

        Console.WriteLine("\n=== PERFORMANCE ANALYSIS ===\n");
        Console.WriteLine($"Total Time: {totalElapsed * 1000:F3} ms");
        Console.WriteLine($"CPU Frequency: ~{_cpuFrequency / 1_000_000:F0} MHz");
        if (totalBytes > 0)
        {
            var megabytes = totalBytes / (1024.0 * 1024.0);
            var throughput = megabytes / totalElapsed;
            Console.WriteLine($"Throughput: {throughput:F2} MB/s");
        }
        Console.WriteLine();

        var sortedZones = _zones.Values.OrderByDescending(z => z.ElapsedTicks).ToList();
        var totalTicks = sortedZones.Sum(z => z.ElapsedTicks);

        Console.WriteLine($"{"Zone",-30} {"Time (ms)",12} {"Percent",8} {"Hit Count",12} {"Avg (us)",12}");
        Console.WriteLine(new string('-', 85));

        foreach (var zone in sortedZones)
        {
            var ms = zone.ElapsedTicks / (double)Stopwatch.Frequency * 1000.0;
            var percent = totalTicks > 0 ? (zone.ElapsedTicks / (double)totalTicks * 100.0) : 0;
            var avgUs = zone.HitCount > 0 ? (ms * 1000.0 / zone.HitCount) : 0;

            Console.WriteLine($"{zone.Name,-30} {ms,12:F3} {percent,7:F2}% {zone.HitCount,12:N0} {avgUs,12:F3}");
        }

        Console.WriteLine();
    }

    private static ulong EstimateCpuFrequency()
    {
        // Estimate by running a timing test
        var sw = Stopwatch.StartNew();
        var startTicks = (ulong)sw.ElapsedTicks;
        Thread.Sleep(100);
        var endTicks = (ulong)sw.ElapsedTicks;
        sw.Stop();

        var elapsed = endTicks - startTicks;
        var seconds = sw.Elapsed.TotalSeconds;

        return (ulong)(elapsed / seconds);
    }
}

public class ProfileZone(string name) : IDisposable
{
    public string Name { get; } = name;
    public long ElapsedTicks { get; private set; }
    public long HitCount { get; private set; }

    private long _startTicks;

    public void Begin()
    {
        _startTicks = Stopwatch.GetTimestamp();
    }

    public void Dispose()
    {
        var endTicks = Stopwatch.GetTimestamp();
        ElapsedTicks += endTicks - _startTicks;
        HitCount++;
    }
}
