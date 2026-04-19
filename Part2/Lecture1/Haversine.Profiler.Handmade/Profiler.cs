namespace Haversine.Profiler.Handmade;

public class Zone(string name) : IDisposable
{
    public ulong ElapsedTicks = 0;
    public string Name = name;

    private readonly ulong startTicks = CpuTimer.CpuTimer.ReadTimestampCounter();

    public void Dispose()
    {
        var endTicks = CpuTimer.CpuTimer.ReadTimestampCounter();
        ElapsedTicks = endTicks - startTicks;
    }
}

public class Profiler
{
    private readonly List<Zone> _zones;
    private readonly double _timerFrequency;

    public Profiler()
    {
        _timerFrequency = CpuTimer.CpuTimer.GetTimerFrequency();
    }

    public Print()
    {
    }
}
