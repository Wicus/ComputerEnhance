namespace HaversineInputJson;

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

public class HaversineGenerator
{
    public static List<Pair> GetPairs(int numberOfPairs, int seed)
    {
        var pairs = new List<Pair>();
        var random = new Random(seed);

        int fullBlocks = numberOfPairs / 64;
        int remainder = numberOfPairs % 64;

        for (int i = 0; i < fullBlocks; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                var x0 = random.NextDouble() * 360 - 180;
                var x1 = random.NextDouble() * 360 - 180;
                var y0 = random.NextDouble() * 180 - 90;
                var y1 = random.NextDouble() * 180 - 90;

                pairs.Add(
                    new Pair
                    {
                        x0 = x0,
                        x1 = x1,
                        y0 = y0,
                        y1 = y1,
                    }
                );
            }
        }

        for (int j = 0; j < remainder; j++)
        {
            var x0 = random.NextDouble() * 360 - 180;
            var x1 = random.NextDouble() * 360 - 180;
            var y0 = random.NextDouble() * 180 - 90;
            var y1 = random.NextDouble() * 180 - 90;

            pairs.Add(
                new Pair
                {
                    x0 = x0,
                    x1 = x1,
                    y0 = y0,
                    y1 = y1,
                }
            );
        }

        return pairs;
    }

    public static double GetSum(List<Pair> pairs)
    {
        var sum = 0.0;
        foreach (var pair in pairs)
        {
            sum += ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, 6372.8);
        }

        sum /= pairs.Count;
        return sum;
    }

    private static double ReferenceHaversine(
        double x0,
        double y0,
        double x1,
        double y1,
        double earthRadius
    )
    {
        var lat1 = y0;
        var lat2 = y1;
        var lon1 = x0;
        var lon2 = x1;

        var dLat = RadiansFromDegrees(lat2 - lat1);
        var dLon = RadiansFromDegrees(lon2 - lon1);
        lat1 = RadiansFromDegrees(lat1);
        lat2 = RadiansFromDegrees(lat2);

        var a =
            Square(Math.Sin(dLat / 2.0))
            + Math.Cos(lat1) * Math.Cos(lat2) * Square(Math.Sin(dLon / 2));
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
