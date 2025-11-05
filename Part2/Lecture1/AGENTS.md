# Repository Guidelines

## Project Structure & Module Organization
- `Haversine.sln`: solution.
- `Haversine.App`: CLI (generate/parse/benchmark).
- `Haversine.Parser`: JSON tokenizer/parser.
- `Haversine.Generator`: data maker.
- `Haversine.Profiler`: timing helpers → `Haversine.Rdtsc`.
- `Haversine.Rdtsc`: P/Invoke to `rdtsc.dll` (Windows only).
- `output/`: generated data (e.g., `haversine.json`).

## Build, Test, and Development Commands
- Restore: `dotnet restore`
- Build: `dotnet build Haversine.sln -c Release`
- Run generate: `dotnet run --project Haversine.App -- generate --pairs 1000000 --seed 42`
- Run parse: `dotnet run --project Haversine.App -- parse`
- Benchmark: `dotnet run --project Haversine.App -- benchmark parse` (best on Windows)

## Coding Style (Casey-ish)
- Clarity > clever. Do the simple thing.
- 4-space indent. Allman braces (new line).
- One idea per function; short, local, testable.
- Names: PascalCase types/methods; camelCase locals/params; no leading underscores.
- Data-oriented: prefer plain structs/classes, composition over inheritance; avoid interfaces until needed.
- No layers you don’t use. Wire code manually; avoid DI frameworks.
- Explicit data flow: pass sizes/counts; no hidden work.
- Errors: assert invariants; throw only at boundaries; never swallow.
- Perf first-class: avoid heap in hot paths; prefer stack/`Span<T>`; avoid LINQ/allocs in tight loops; measure with `Profiler`.
- Files match primary type; keep file size modest.

## Testing Guidelines
- Use xUnit. New project: `dotnet new xunit -n Haversine.Tests`.
- Name `FooTests.cs`; methods `[Fact]`/`[Theory]`.
- Test parser/generator determinism and error paths.
- Run: `dotnet test`.

## Commit & Pull Request Guidelines
- Commits: `feat|fix|chore(scope): subject` (short, factual).
- Describe what/why; include perf numbers when relevant (baseline → new).
- PRs: steps to repro, CLI examples, link issues; show `dotnet build` clean.

## Cross-Platform & Timing
- .NET 9 runs on Windows/Linux/macOS.
- OS switch done: `CpuTimer` P/Invoke loads logical `rdtsc` (dll/so/dylib).
- Linux build: `bash Haversine.Rdtsc/build_rdtsc.sh` → `Haversine.Rdtsc/librdtsc.so` auto-copied to output.
- Windows: keep `Haversine.Rdtsc/rdtsc.dll` (already copied).
- macOS (optional): build `librdtsc.dylib`, drop next to project, it will copy.
- Keep `output/` small and reproducible; don’t commit large blobs.
