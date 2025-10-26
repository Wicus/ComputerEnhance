using Haversine.Profiler;

namespace Haversine.Parser;

public struct CoordinatePair
{
    public double X0;
    public double Y0;
    public double X1;
    public double Y1;
}

public class Parser(IProfiler profiler)
{
    private const double EarthRadiusKm = 6372.8;

    public void Parse(string path)
    {
        using var parseZone = profiler.BeginZone("Parse");

        JsonValue json;
        using (var fileStream = new FileStream(path, FileMode.Open, FileAccess.Read))
        using (var reader = new StreamReader(fileStream))
        {
            using var jsonParseZone = profiler.BeginZone("JsonParse");
            var parser = new JsonParser(profiler, reader);
            json = parser.Parse();
        }

        double sum = 0.0;
        long count = 0;

        if (json.Type == JsonValueType.Object)
        {
            using var processZone = profiler.BeginZone("Process");
            var obj = json.AsObject();
            if (obj.TryGetValue("pairs", out var pairsValue) && pairsValue.Type == JsonValueType.Array)
            {
                var pairs = pairsValue.AsArray();
                using (var loopZone = profiler.BeginZone("CoordinateLoop"))
                {
                    foreach (var pair in pairs)
                    {

                        if (pair.Type != JsonValueType.Object)
                        {
                            continue;
                        }

                        var pairObj = pair.AsObject();
                        if (pairObj.TryGetValue("x0", out var x0Val) && x0Val.Type == JsonValueType.Number &&
                            pairObj.TryGetValue("y0", out var y0Val) && y0Val.Type == JsonValueType.Number &&
                            pairObj.TryGetValue("x1", out var x1Val) && x1Val.Type == JsonValueType.Number &&
                            pairObj.TryGetValue("y1", out var y1Val) && y1Val.Type == JsonValueType.Number)
                        {
                            var x0 = x0Val.AsNumber();
                            var y0 = y0Val.AsNumber();
                            var x1 = x1Val.AsNumber();
                            var y1 = y1Val.AsNumber();

                            double distance;
                            using (var haversineZone = profiler.BeginZone("Haversine"))
                            {
                                distance = ComputeHaversine(x0, y0, x1, y1);
                            }

                            sum += distance;
                            count++;
                        }
                    }
                }
            }
        }

        var haversineSum = count > 0 ? sum / count : 0.0;

        Console.WriteLine($"Pair Count: {count}");
        Console.WriteLine($"Haversine Sum: {haversineSum}");
    }

    public void PrintResults(string path)
    {
        var fileInfo = new FileInfo(path);
        profiler.PrintResults(fileInfo.Length);
    }

    private double ComputeHaversine(double x0, double y0, double x1, double y1)
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

        var result = EarthRadiusKm * c;

        return result;
    }

    private double Square(double num)
    {
        double result = num * num;
        return result;
    }

    private double RadiansFromDegrees(double degrees)
    {
        double result = 0.01745329251994329577 * degrees;
        return result;
    }
}
