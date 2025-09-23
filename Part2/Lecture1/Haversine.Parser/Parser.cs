using System.Diagnostics;

namespace Haversine.Parser;


public class Parser
{
    public static void Parse(string dataFile)
    {
        using var reader = new StreamReader(dataFile);

        var watch = new Stopwatch();

        watch.Start();

        while (!reader.EndOfStream && watch.ElapsedMilliseconds < 2000)
        {
            Console.Write((char)reader.Read());
        }

        watch.Stop();
    }
}
