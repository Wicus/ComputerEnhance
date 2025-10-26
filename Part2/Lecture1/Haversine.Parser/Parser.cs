using System.Globalization;

namespace Haversine.Parser;

public struct CoordinatePair
{
    public double X0;
    public double Y0;
    public double X1;
    public double Y1;
}

public static class Parser
{
    private const double EarthRadiusKm = 6372.8;
    private const int MaxPropertyLength = 16;
    private const int MaxNumberLength = 64;

    public static void Parse(string path)
    {
        using var fileStream = new FileStream(path, FileMode.Open, FileAccess.Read);
        using var reader = new StreamReader(fileStream);

        double sum = 0.0;
        long count = 0;

        CoordinatePair coords = new()
        {
            X0 = double.NaN,
            Y0 = double.NaN,
            X1 = double.NaN,
            Y1 = double.NaN
        };

        Span<char> propertyBuffer = stackalloc char[MaxPropertyLength];

        int next;
        while ((next = reader.Read()) != -1)
        {
            var ch = (char)next;
            switch (ch)
            {
                case '{':
                    SkipWhiteSpace(reader);
                    coords.X0 = coords.Y0 = coords.X1 = coords.Y1 = double.NaN;
                    break;

                case '}':
                    SkipWhiteSpace(reader);
                    if (reader.Peek() == -1)
                    {
                        break;
                    }
                    if (!double.IsNaN(coords.X0) && !double.IsNaN(coords.Y0) && !double.IsNaN(coords.X1) && !double.IsNaN(coords.Y1))
                    {
                        sum += ComputeHaversine(coords.X0, coords.Y0, coords.X1, coords.Y1);
                        count++;
                    }
                    SkipWhiteSpace(reader);
                    break;

                case '"':
                    {
                        int propLength = 0;
                        while ((next = reader.Read()) != -1)
                        {
                            var c = (char)next;
                            if (c == '"')
                            {
                                break;
                            }
                            if (propLength >= propertyBuffer.Length)
                            {
                                throw new InvalidDataException("Property name exceeds parser buffer.");
                            }
                            propertyBuffer[propLength++] = c;
                        }

                        if (next == -1)
                        {
                            throw new InvalidDataException("Unexpected end of file while reading property name.");
                        }

                        ReadOnlySpan<char> propertyName = propertyBuffer[..propLength];

                        SkipWhiteSpace(reader);
                        Expect(reader, ':');
                        SkipWhiteSpace(reader);

                        if (propertyName.SequenceEqual("pairs"))
                        {
                            Expect(reader, '[');
                            SkipWhiteSpace(reader);
                        }
                        else if (propertyName.SequenceEqual("x0"))
                        {
                            coords.X0 = ReadDouble(reader);
                        }
                        else if (propertyName.SequenceEqual("y0"))
                        {
                            coords.Y0 = ReadDouble(reader);
                        }
                        else if (propertyName.SequenceEqual("x1"))
                        {
                            coords.X1 = ReadDouble(reader);
                        }
                        else if (propertyName.SequenceEqual("y1"))
                        {
                            coords.Y1 = ReadDouble(reader);
                        }
                    }
                    break;

                case '[':
                    SkipWhiteSpace(reader);
                    break;

                case ']':
                    SkipWhiteSpace(reader);
                    break;
            }
        }

        var haversineSum = count > 0 ? sum / count : 0.0;

        Console.WriteLine($"Pair Count: {count}");
        Console.WriteLine($"Haversine Sum: {haversineSum}");
    }

    private static void Expect(StreamReader reader, char expected)
    {
        var next = reader.Read();
        if (next == -1 || (char)next != expected)
        {
            throw new InvalidDataException($"Expected '{expected}' while parsing JSON.");
        }
    }

    private static void SkipWhiteSpace(StreamReader reader)
    {
        while (true)
        {
            var next = reader.Peek();
            if (next == -1 || !char.IsWhiteSpace((char)next))
            {
                return;
            }
            reader.Read();
        }
    }

    private static double ReadDouble(StreamReader reader)
    {
        Span<char> numberBuffer = stackalloc char[MaxNumberLength];
        var length = 0;

        while (true)
        {
            var peek = reader.Peek();
            if (peek == -1 || !IsNumericChar(peek))
            {
                break;
            }

            if (length >= numberBuffer.Length)
            {
                throw new InvalidDataException("Numeric literal exceeds parser buffer.");
            }

            numberBuffer[length++] = (char)reader.Read();
        }

        if (length == 0)
        {
            throw new InvalidDataException("Expected numeric literal.");
        }

        return double.Parse(numberBuffer[..length], NumberStyles.Float | NumberStyles.AllowLeadingSign, CultureInfo.InvariantCulture);
    }

    private static bool IsNumericChar(int ch)
    {
        return ch >= '0' && ch <= '9' || ch is '+' or '-' or '.' or 'e' or 'E';
    }

    private static double ComputeHaversine(double x0, double y0, double x1, double y1)
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

