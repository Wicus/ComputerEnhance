using Haversine.Generator;

var (pairs, seed) = ParseArgs(args);

Generator.WriteJson(pairs, seed);

// Private

static (int pairs, int seed) ParseArgs(string[] args)
{
    if (args[0] == null || args[1] == null)
    {
        throw new ArgumentException("Missing arguments");
    }

    var pairs = int.Parse(args[0]);
    if (pairs <= 0)
    {
        throw new ArgumentException("Invalid number of pairs");
    }
    if (pairs > 10_000_000)
    {
        throw new ArgumentException("Number of pairs too large");
    }

    var seed = int.Parse(args[1]);
    if (seed < 0)
    {
        throw new ArgumentException("Invalid seed");
    }

    return (pairs, seed);
}