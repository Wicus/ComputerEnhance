using Haversine.Profiler;
using Microsoft.Extensions.DependencyInjection;
using HaversineParser = Haversine.Parser.Parser;
using HaversineProfiler = Haversine.Profiler.Profiler;

namespace Haversine.App;

class Args {
    /// <summary>
    /// The type of command to execute (generate, parse, benchmark)
    /// </summary>
    public CommandType CommandType { get; set; }

    /// <summary>
    /// The file path to read from or write to (for generate and parse commands)
    /// </summary>
    public string FilePath { get; set; } = string.Empty;

    /// <summary>
    /// Parses the command-line arguments and populates the properties of this class.
    /// Returns true if parsing was successful, false otherwise.
    /// </summary>
    public bool TryParse(string[] sourceArgs)
    {
        if (sourceArgs.Length == 0)
            return false;

        // FilePath
        var projectDir = Directory.GetCurrentDirectory();
        var filePath = Path.Combine(projectDir, "output", "haversine.json");

        // CommandType
        var command = sourceArgs[0];
        if (!DescriptionEnum.TryParse<CommandType>(command, ignoreCase: true, out var commandType))
            return false;

        FilePath = filePath;
        CommandType = commandType;

        return true;
    }
}

static class Program
{
    private const long MaxPairs = 100_000_000;

    private static int Main(string[] args)
    {
        var parsedArgs = new Args();
        if (!parsedArgs.TryParse(args))
        {
            Console.WriteLine("Error: Invalid arguments");
            PrintUsage();
            return 1;
        }

        using var services = SetupServiceProvider();

        return parsedArgs.CommandType switch
        {
            CommandType.Generate => HandleGenerate(args, parsedArgs.FilePath),
            CommandType.Parse => HandleParse(services, parsedArgs.FilePath),
            CommandType.Benchmark => HandleBenchmark(args),
            _ => InvalidCommand(args[0]),
        };
    }

    private static ServiceProvider SetupServiceProvider()
    {
        var serviceCollection = new ServiceCollection();
        serviceCollection.AddSingleton<IProfiler, HaversineProfiler>();
        serviceCollection.AddTransient<HaversineParser>();
        return serviceCollection.BuildServiceProvider();
    }

    private static int HandleGenerate(string[] args, string filePath)
    {
        if (args.Length < 5)
        {
            Console.WriteLine("Error: generate requires --pairs <count> --seed <value>");
            return 1;
        }

        long pairs = 0;
        int seed = 0;

        for (var i = 0; i < args.Length; i++)
        {
            var arg = args[i];
            if (arg == "--pairs")
            {
                if (!long.TryParse(args[++i], out pairs))
                {
                    Console.WriteLine($"Error: Invalid pairs value '{args[i]}'");
                    return 1;
                }
            }
            else if (arg == "--seed")
            {
                if (!int.TryParse(args[++i], out seed))
                {
                    Console.WriteLine($"Error: Invalid seed value '{args[i]}'");
                    return 1;
                }
            }
        }

        if (pairs <= 0)
        {
            Console.WriteLine("Error: Number of pairs must be greater than 0");
            return 1;
        }

        if (pairs > MaxPairs)
        {
            Console.WriteLine($"Error: Number of pairs cannot exceed {MaxPairs:N0}");
            return 1;
        }

        if (seed < 0)
        {
            Console.WriteLine("Error: Seed must be non-negative");
            return 1;
        }

        Generator.Generator.WriteJson(filePath, pairs, seed);
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

    private static int InvalidCommand(string command)
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
