namespace Haversine.Generator;

public class Pair
{
    public double x0;
    public double x1;
    public double y0;
    public double y1;
}

public class JsonData
{
    public List<Pair> pairs = [];
}

public class Generator
{
    public static void WriteJson(string dataFile, int pairs, int seed)
    {
        using var writer = new StreamWriter(dataFile);

        writer.WriteLine('{');
        writer.WriteLine("  \"pairs\": [");

        var pairsEnumerator = GetPairsEnumerator(pairs, seed);
        var pairsCount = 0;
        var sum = 0.0;
        var isFirst = true;
        foreach (var pair in pairsEnumerator)
        {
            if (!isFirst)
            {
                writer.WriteLine(',');
            }

            writer.WriteLine("    {");
            writer.WriteLine($"      \"x0\": {pair.x0},");
            writer.WriteLine($"      \"x1\": {pair.x1},");
            writer.WriteLine($"      \"y0\": {pair.y0},");
            writer.WriteLine($"      \"y1\": {pair.y1}");
            writer.Write("    }");

            sum += ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, 6372.8);
            pairsCount++;
            isFirst = false;
        }

        writer.WriteLine("  ]");
        writer.WriteLine('}');

        Console.WriteLine("Pair Count: {0}", pairsCount);
        Console.WriteLine("Expected Sum: {0}", sum / pairsCount);
    }

    public static IEnumerable<Pair> GetPairsEnumerator(int numberOfPairs, int seed)
    {
        var random = new Random(seed);
        var maxAllowedX = 180.0;
        var maxAllowedY = 90.0;
        var xRadius = maxAllowedX / 64;
        var yRadius = maxAllowedY / 64;

        for (var i = 0; i < numberOfPairs; i += 64)
        {
            var xCenter = Math.Clamp(GetRandomX(random), -maxAllowedX, maxAllowedX);
            var yCenter = Math.Clamp(GetRandomY(random), -maxAllowedY, maxAllowedY);

            for (var j = 0; j < 64 && i + j < numberOfPairs; j++)
            {
                var x0 = RandomDegree(random, xCenter, xRadius, maxAllowedX);
                var x1 = RandomDegree(random, x0 + xCenter, xRadius, maxAllowedX);
                var y0 = RandomDegree(random, yCenter, yRadius, maxAllowedY);
                var y1 = RandomDegree(random, y0 + yCenter, yRadius, maxAllowedY);

                yield return new Pair
                {
                    x0 = x0,
                    x1 = x1,
                    y0 = y0,
                    y1 = y1,
                };
            }
        }
    }

    public static double ReferenceHaversine(double x0, double y0, double x1, double y1, double earthRadius)
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

    private static double GetRandomX(Random random)
    {
        return random.NextDouble() * 360 - 180;
    }

    private static double GetRandomY(Random random)
    {
        return random.NextDouble() * 180 - 90;
    }

    private static double RandomDegree(Random random, double center, double radius, double maxAllowed)
    {
        var minValue = Math.Max(center - radius, -maxAllowed);
        var maxValue = Math.Min(center + radius, maxAllowed);

        var range = maxValue - minValue;
        var randomDegreeInRange = minValue + random.NextDouble() * range;
        return randomDegreeInRange;
    }
}
