namespace Haversine.Tests;

public sealed class CpuTimerTests
{
    [Fact]
    public void CallToQueryPerformanceQueryWorks()
    {
        Console.WriteLine($"Estimated CPU timer frequency: {CpuTimer.CpuTimer.EstimateFrequency()} Hz");
    }
}
