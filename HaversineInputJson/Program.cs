using System.Text.Json;

namespace HaversineInputJson;

class Program
{
    static void Main(string[] args)
    {
        var parsedArgs = new ArgsParser(args);
        using var writer = new StreamWriter("data.json");

        writer.WriteLine('{');
        writer.WriteLine("  \"pairs\": [");

        var pairs = HaversineGenerator.GetPairsEnumerator(parsedArgs.NumberOfPairs, parsedArgs.Seed);
        var pairsCount = 0;
        var sum = 0.0;
        var isFirst = true;
        foreach (var pair in pairs)
        {
            if (!isFirst)
            {
                writer.Write(',');
            }

            writer.WriteLine("  {");
            writer.WriteLine($"     \"x0\": {pair.x0},");
            writer.WriteLine($"     \"x1\": {pair.x1},");
            writer.WriteLine($"     \"y0\": {pair.y0},");
            writer.WriteLine($"     \"y1\": {pair.y1}");
            writer.Write("  }");

            sum += HaversineGenerator.ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, 6372.8);
            pairsCount++;
            isFirst = false;
        }

        writer.WriteLine("  ]");
        writer.WriteLine('}');

        Console.WriteLine("Pair Count: {0}", pairsCount);
        Console.WriteLine("Expected Sum: {0}", sum / pairsCount);
    }
}
