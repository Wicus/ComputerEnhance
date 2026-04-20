using Haversine.Profiler;
using Microsoft.Extensions.DependencyInjection;
using HaversineParser = Haversine.Parser.Parser;
using HaversineProfiler = Haversine.Profiler.Profiler;

namespace Haversine.App;

static class Program
{
    private static int Main(string[] args)
    {
        var parsedArgs = new Args(args);
        var parseResult = parsedArgs.TryParse();
        if (!parseResult)
        {
            Console.WriteLine($"Error: {parseResult.Error}");
            PrintUsage();
            return 1;
        }

        using var services = SetupServiceProvider();

        return parsedArgs.CommandType switch
        {
            CommandType.Generate => HandleGenerate(parsedArgs),
            CommandType.Parse => HandleParse(services, parsedArgs.FilePath),
            CommandType.Benchmark => HandleBenchmark(args),
            _ => InvalidCommand(parsedArgs.CommandType.ToString()),
        };
    }

    private static ServiceProvider SetupServiceProvider()
    {
        var serviceCollection = new ServiceCollection();
        serviceCollection.AddSingleton<IProfiler, HaversineProfiler>();
        serviceCollection.AddTransient<HaversineParser>();
        return serviceCollection.BuildServiceProvider();
    }

    private static int HandleGenerate(Args parsedArgs)
    {
        var result = parsedArgs.TryParseGenerate();
        if (!result)
        {
            Console.WriteLine($"Error: {result.Error}");
            return 1;
        }

        Generator.Generator.WriteJson(parsedArgs.FilePath, parsedArgs.Pairs, parsedArgs.Seed);
        return 0;
    }

    private static int HandleParse(ServiceProvider services, string filePath)
    {
        if (!File.Exists(filePath))
        {
            Console.WriteLine($"Error: File not found: {filePath}");
            return 1;
        }

        var parser = services.GetRequiredService<HaversineParser>();
        parser.Parse(filePath);
        parser.PrintResults(filePath);
        return 0;
    }

    private static int InvalidCommand(string? command)
    {
        Console.WriteLine($"Error: Unknown command '{command}'");
        PrintUsage();
        return 1;
    }

    private static int HandleBenchmark(string[] args)
    {
        if (args.Length < 2)
        {
            Console.WriteLine("Usage: haversine benchmark <operation>");
            Console.WriteLine("  Operations: parse, generate");
            return 1;
        }

        var operation = args[1];

        switch (operation)
        {
            case "parse":
            {
                var projectDir = Directory.GetCurrentDirectory();
                var filePath = Path.Combine(projectDir, "output", "haversine.json");
                if (!File.Exists(filePath))
                {
                    Console.WriteLine($"Error: File not found: {filePath}");
                    Console.WriteLine("Generate a file first with: haversine generate --pairs 1000000 --seed 42");
                    return 1;
                }

                // Parser.Parser.Benchmark(filePath);
                return 0;
            }

            case "generate":
            {
                // Benchmark.Run("Generate Haversine Data", () => Generator.Generator.WriteJson(filePath, pairs, seed));
                return 0;
            }

            default:
                Console.WriteLine($"Unknown operation: {operation}");
                return 1;
        }
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Haversine calculator - generates or parses coordinate data");
        Console.WriteLine();
        Console.WriteLine("Usage:");
        Console.WriteLine("  dotnet run -- generate --pairs <count> --seed <value>");
        Console.WriteLine("  dotnet run -- parse");
        Console.WriteLine("  dotnet run -- benchmark <operation>");
        Console.WriteLine();
        Console.WriteLine("Commands:");
        Console.WriteLine("  generate  - Generate coordinate pairs");
        Console.WriteLine("  parse     - Parse and calculate distances from generated file");
        Console.WriteLine("  benchmark - Run performance benchmarks (parse: IEnumerable vs List)");
        Console.WriteLine();
        Console.WriteLine("Examples:");
        Console.WriteLine("  dotnet run -- generate --pairs 1000000 --seed 1337");
        Console.WriteLine("  dotnet run -- parse");
    }
}
