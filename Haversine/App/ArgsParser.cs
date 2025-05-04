namespace Haversine.App;

public class ArgsParser
{
    public int NumberOfPairs { get; set; }
    public int Seed { get; set; }

    public ArgsParser(string[] args)
    {
        try
        {
            if (args[0] == null || args[1] == null)
            {
                throw new ArgumentException("Missing arguments");
            }

            NumberOfPairs = int.Parse(args[0]);
            if (NumberOfPairs <= 0)
            {
                throw new ArgumentException("Invalid number of pairs");
            }

            Seed = int.Parse(args[1]);
            if (Seed < 0)
            {
                throw new ArgumentException("Invalid seed");
            }
        }
        catch
        {
            throw;
        }
    }
}
