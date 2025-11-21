# Haversine.CpuTimer

Minimal rdtsc shim plus harness for debugging and frequency checks.

## Build the native DLL

```powershell
pwsh -File Haversine.CpuTimer/build_rdtsc.ps1
```

## Run the managed harness

```bash
dotnet run --project Haversine.CpuTimer.Harness -- 10
```

- Argument = number of samples (defaults to 5 if omitted).
- Set `Haversine.CpuTimer.Harness` as the startup project in Visual Studio, enable native code debugging, and you can step straight into `rdtsc.cpp`.

## Run the native (C/C++) harness

```powershell
pwsh -File Haversine.CpuTimer/build_rdtsc_harness.ps1
./Haversine.CpuTimer/rdtsc_harness.exe 10
```

- Harness links directly with `rdtsc.cpp`, so any breakpoint hits instantly under a native debugger (e.g. RAB).
- Omit the numeric argument to default to five 10 ms samples.
