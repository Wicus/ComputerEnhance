using System.Text.Json;

namespace HaversineInputJson;

class Program
{
    private static readonly JsonSerializerOptions options = new() { IncludeFields = true, WriteIndented = true };

    static void Main(string[] args)
    {
        var parsedArgs = new ArgsParser(args);
        var pairs = HaversineGenerator.GetPairs(parsedArgs.NumberOfPairs, parsedArgs.Seed);
        var sum = HaversineGenerator.GetSum(pairs);

        File.WriteAllText("data.json", JsonSerializer.Serialize(new JsonData { pairs = pairs }, options));

        Console.WriteLine("Pair Count: {0}", pairs.Count);
        Console.WriteLine("Expected Sum: {0}", sum);
    }
}
