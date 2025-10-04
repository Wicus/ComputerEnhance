namespace Haversine.App;

internal static class Program
{
    private const long MaxPairs = 10_000_000_000;

    private static int Main(string[] args)
    {
        if (args.Length == 0)
        {
            PrintUsage();
            return 1;
        }

        var command = args[0];
        var filePath = Path.Combine(AppContext.BaseDirectory, "output", "haversine.json");

        return command switch
        {
            "generate" => HandleGenerate(args, filePath),
            "parse" => HandleParse(filePath),
            "benchmark" => HandleBenchmark(args),
            _ => InvalidCommand(command)
        };
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

        for (int i = 1; i < args.Length - 1; i++)
        {
            if (args[i] == "--pairs")
            {
                if (!long.TryParse(args[i + 1], out pairs))
                {
                    Console.WriteLine($"Error: Invalid pairs value '{args[i + 1]}'");
                    return 1;
                }
                i++;
            }
            else if (args[i] == "--seed")
            {
                if (!int.TryParse(args[i + 1], out seed))
                {
                    Console.WriteLine($"Error: Invalid seed value '{args[i + 1]}'");
                    return 1;
                }
                i++;
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

    private static int HandleParse(string filePath)
    {
        if (!File.Exists(filePath))
        {
            Console.WriteLine($"Error: File not found: {filePath}");
            return 1;
        }

        Parser.Parser.Parse(filePath);
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
                var filePath = Path.Combine(AppContext.BaseDirectory, "output", "haversine.json");
                if (!File.Exists(filePath))
                {
                    Console.WriteLine($"Error: File not found: {filePath}");
                    Console.WriteLine("Generate a file first with: haversine generate --pairs 5_000_000_000 --seed 42");
                    return 1;
                }

                Benchmark.Run("Parse Haversine Data", () => Parser.Parser.Parse(filePath));
                return 0;
            }

            case "generate":
            {
                var filePath = Path.Combine(AppContext.BaseDirectory, "output", "haversine.json");
                const long pairs = 5_000_000_000;
                const int seed = 42;

                Benchmark.Run("Generate Haversine Data", () => Generator.Generator.WriteJson(filePath, pairs, seed));
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
        Console.WriteLine("  haversine generate --pairs <count> --seed <value>");
        Console.WriteLine("  haversine parse");
        Console.WriteLine("  haversine benchmark <operation>");
        Console.WriteLine();
        Console.WriteLine("Benchmark operations:");
        Console.WriteLine("  parse     - Benchmark parsing performance");
        Console.WriteLine("  generate  - Benchmark generation performance");
    }
}
