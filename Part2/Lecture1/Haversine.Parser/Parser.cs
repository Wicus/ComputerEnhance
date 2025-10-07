using System.Diagnostics;
using System.Globalization;

namespace Haversine.Parser;

public class Parser
{
    private const double EarthRadius = 6372.8;

    public static void Parse(string dataFile)
    {
        var watch = Stopwatch.StartNew();

        using var reader = new StreamReader(dataFile);
        var pairs = ParsePairs(reader);
        var sum = 0.0;
        var count = 0L;

        foreach (var (x0, y0, x1, y1) in pairs)
        {
            sum += ReferenceHaversine(x0, y0, x1, y1, EarthRadius);
            count++;
        }

        watch.Stop();

        var average = count > 0 ? sum / count : 0.0;

        Console.WriteLine($"Pair Count: {count}");
        Console.WriteLine($"Haversine Sum: {average}");
        Console.WriteLine($"Time: {watch.ElapsedMilliseconds}ms");
    }

    public static void Benchmark(string dataFile, int iterations = 10)
    {
        Console.WriteLine("=== True Streaming (Disk → Parse → Process) vs Memory (Load All) ===\n");

        // Warmup
        BenchmarkTrueStreaming(dataFile);
        BenchmarkMemoryBased(dataFile);

        var streamingTimes = new long[iterations];
        var memoryTimes = new long[iterations];

        for (int i = 0; i < iterations; i++)
        {
            streamingTimes[i] = BenchmarkTrueStreaming(dataFile);
            memoryTimes[i] = BenchmarkMemoryBased(dataFile);
        }

        var streamingAvg = streamingTimes.Average();
        var memoryAvg = memoryTimes.Average();
        var streamingMin = streamingTimes.Min();
        var memoryMin = memoryTimes.Min();

        Console.WriteLine($"\nTrue Streaming (StreamReader):");
        Console.WriteLine($"  Best: {streamingMin}ms");
        Console.WriteLine($"  Avg:  {streamingAvg:F2}ms");

        Console.WriteLine($"\nMemory-Based (ReadToEnd + ToList):");
        Console.WriteLine($"  Best: {memoryMin}ms");
        Console.WriteLine($"  Avg:  {memoryAvg:F2}ms");

        Console.WriteLine($"\nDifference: {memoryMin - streamingMin}ms ({(memoryAvg / streamingAvg - 1) * 100:F1}% slower)");
    }

    private static long BenchmarkTrueStreaming(string dataFile)
    {
        var watch = Stopwatch.StartNew();

        using var reader = new StreamReader(dataFile);
        var pairs = ParsePairs(reader);
        var sum = 0.0;
        var count = 0L;

        foreach (var (x0, y0, x1, y1) in pairs)
        {
            sum += ReferenceHaversine(x0, y0, x1, y1, EarthRadius);
            count++;
        }

        watch.Stop();
        return watch.ElapsedMilliseconds;
    }

    private static long BenchmarkMemoryBased(string dataFile)
    {
        var watch = Stopwatch.StartNew();

        using var reader = new StreamReader(dataFile);
        var json = reader.ReadToEnd();

        // Simulate old approach: parse entire JSON into List
        var pairs = ParsePairsFromString(json).ToList();
        var sum = 0.0;
        var count = 0L;

        foreach (var (x0, y0, x1, y1) in pairs)
        {
            sum += ReferenceHaversine(x0, y0, x1, y1, EarthRadius);
            count++;
        }

        watch.Stop();
        return watch.ElapsedMilliseconds;
    }

    // Old string-based parser for comparison
    private static IEnumerable<(double x0, double y0, double x1, double y1)> ParsePairsFromString(string json)
    {
        var index = 0;

        if (!SkipToString(json, ref index, "\"pairs\""))
            yield break;

        if (!SkipToString(json, ref index, "["))
            yield break;

        index++;

        while (index < json.Length)
        {
            SkipWhitespaceString(json, ref index);

            if (index >= json.Length || json[index] == ']')
                break;

            if (json[index] != '{')
                break;

            index++;

            var pair = ParsePairFromString(json, ref index);
            if (pair.HasValue)
                yield return pair.Value;

            SkipWhitespaceString(json, ref index);

            if (index < json.Length && json[index] == ',')
                index++;
        }
    }

    private static (double x0, double y0, double x1, double y1)? ParsePairFromString(string json, ref int index)
    {
        double? x0 = null, y0 = null, x1 = null, y1 = null;

        while (index < json.Length)
        {
            SkipWhitespaceString(json, ref index);

            if (index >= json.Length || json[index] == '}')
            {
                index++;
                break;
            }

            if (json[index] == '"')
            {
                index++;
                var propertyName = ReadUntilString(json, ref index, '"');
                index++;

                SkipWhitespaceString(json, ref index);

                if (index >= json.Length || json[index] != ':')
                    return null;

                index++;
                SkipWhitespaceString(json, ref index);

                var numberStr = ReadNumberString(json, ref index);
                if (double.TryParse(numberStr, NumberStyles.Float, CultureInfo.InvariantCulture, out var value))
                {
                    switch (propertyName)
                    {
                        case "x0": x0 = value; break;
                        case "y0": y0 = value; break;
                        case "x1": x1 = value; break;
                        case "y1": y1 = value; break;
                    }
                }

                SkipWhitespaceString(json, ref index);

                if (index < json.Length && json[index] == ',')
                    index++;
            }
            else
            {
                index++;
            }
        }

        if (x0.HasValue && y0.HasValue && x1.HasValue && y1.HasValue)
            return (x0.Value, y0.Value, x1.Value, y1.Value);

        return null;
    }

    private static bool SkipToString(string json, ref int index, string target)
    {
        var targetIndex = json.IndexOf(target, index, StringComparison.Ordinal);
        if (targetIndex >= 0)
        {
            index = targetIndex + target.Length;
            return true;
        }
        return false;
    }

    private static void SkipWhitespaceString(string json, ref int index)
    {
        while (index < json.Length && char.IsWhiteSpace(json[index]))
            index++;
    }

    private static string ReadUntilString(string json, ref int index, char delimiter)
    {
        var start = index;
        while (index < json.Length && json[index] != delimiter)
            index++;
        return json.Substring(start, index - start);
    }

    private static string ReadNumberString(string json, ref int index)
    {
        var start = index;
        while (index < json.Length && (char.IsDigit(json[index]) || json[index] == '.' || json[index] == '-' || json[index] == '+' || json[index] == 'e' || json[index] == 'E'))
            index++;
        return json.Substring(start, index - start);
    }

    private static IEnumerable<(double x0, double y0, double x1, double y1)> ParsePairs(StreamReader reader)
    {
        // Skip to "pairs" array
        if (!SkipTo(reader, "\"pairs\""))
        {
            yield break;
        }

        // Skip to the opening bracket
        if (!SkipTo(reader, "["))
        {
            yield break;
        }

        while (true)
        {
            SkipWhitespace(reader);

            int ch = reader.Peek();
            if (ch == -1 || ch == ']')
            {
                break;
            }

            // Expect '{'
            if (ch != '{')
            {
                break;
            }

            reader.Read(); // Consume '{'

            var pair = ParsePair(reader);
            if (pair.HasValue)
            {
                // This yields means we are using streaming
                // We process each pair as soon as we parse it
                // without waiting to load everything into memory
                yield return pair.Value;
            }

            SkipWhitespace(reader);

            // Skip comma if present
            if (reader.Peek() == ',')
            {
                reader.Read();
            }
        }
    }

    private static (double x0, double y0, double x1, double y1)? ParsePair(StreamReader reader)
    {
        double? x0 = null, y0 = null, x1 = null, y1 = null;

        while (true)
        {
            SkipWhitespace(reader);

            int ch = reader.Peek();
            if (ch == -1 || ch == '}')
            {
                reader.Read(); // Consume '}'
                break;
            }

            // Parse property name
            if (ch == '"')
            {
                reader.Read(); // Consume opening quote
                var propertyName = ReadUntil(reader, '"');

                SkipWhitespace(reader);

                // Expect ':'
                if (reader.Peek() != ':')
                {
                    return null;
                }

                reader.Read(); // Consume ':'
                SkipWhitespace(reader);

                // Parse number
                var numberStr = ReadNumber(reader);
                if (double.TryParse(numberStr, NumberStyles.Float, CultureInfo.InvariantCulture, out var value))
                {
                    switch (propertyName)
                    {
                        case "x0": x0 = value; break;
                        case "y0": y0 = value; break;
                        case "x1": x1 = value; break;
                        case "y1": y1 = value; break;
                    }
                }

                SkipWhitespace(reader);

                // Skip comma if present
                if (reader.Peek() == ',')
                {
                    reader.Read();
                }
            }
            else
            {
                reader.Read();
            }
        }

        if (x0.HasValue && y0.HasValue && x1.HasValue && y1.HasValue)
        {
            return (x0.Value, y0.Value, x1.Value, y1.Value);
        }

        return null;
    }

    private static bool SkipTo(StreamReader reader, string target)
    {
        int targetIndex = 0;

        while (true)
        {
            int ch = reader.Read();
            if (ch == -1)
            {
                return false;
            }

            if (ch == target[targetIndex])
            {
                targetIndex++;
                if (targetIndex == target.Length)
                {
                    return true;
                }
            }
            else if (targetIndex > 0)
            {
                // Reset if we had a partial match
                targetIndex = ch == target[0] ? 1 : 0;
            }
        }
    }

    private static void SkipWhitespace(StreamReader reader)
    {
        while (true)
        {
            int ch = reader.Peek();
            if (ch == -1 || !char.IsWhiteSpace((char)ch))
            {
                break;
            }
            reader.Read();
        }
    }

    private static string ReadUntil(StreamReader reader, char delimiter)
    {
        var sb = new System.Text.StringBuilder();

        while (true)
        {
            int ch = reader.Read();
            if (ch == -1 || ch == delimiter)
            {
                break;
            }
            sb.Append((char)ch);
        }

        return sb.ToString();
    }

    private static string ReadNumber(StreamReader reader)
    {
        var sb = new System.Text.StringBuilder();

        while (true)
        {
            int ch = reader.Peek();
            if (ch == -1)
            {
                break;
            }

            char c = (char)ch;
            if (char.IsDigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')
            {
                sb.Append(c);
                reader.Read();
            }
            else
            {
                break;
            }
        }

        return sb.ToString();
    }

    private static double ReferenceHaversine(double x0, double y0, double x1, double y1, double earthRadius)
    {
        var lat1 = y0;
        var lat2 = y1;
        var lon1 = x0;
        var lon2 = x1;

        var dLat = RadiansFromDegrees(lat2 - lat1);
        var dLon = RadiansFromDegrees(lon2 - lon1);
        lat1 = RadiansFromDegrees(lat1);
        lat2 = RadiansFromDegrees(lat2);

        var a = Square(Math.Sin(dLat / 2.0)) + Math.Cos(lat1) * Math.Cos(lat2) * Square(Math.Sin(dLon / 2));
        var c = 2.0 * Math.Asin(Math.Sqrt(a));

        var result = earthRadius * c;

        return result;
    }

    private static double Square(double num)
    {
        double result = num * num;
        return result;
    }

    private static double RadiansFromDegrees(double degrees)
    {
        double result = 0.01745329251994329577f * degrees;
        return result;
    }
}
