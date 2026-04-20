namespace Haversine.App;

/// <summary>
/// Parses and validates CLI arguments. Properties are safe to read only after a successful Result.
/// </summary>
class Args
{
    private readonly List<string> _args;
    private const long MaxPairs = 100_000_000;

    public CommandType CommandType { get; private set; }
    public string FilePath { get; private set; } = string.Empty;
    public long Pairs { get; private set; }
    public int Seed { get; private set; }

    public Args(string[] args)
    {
        _args = args.ToList();
    }

    /// <summary>
    /// Parses the command type and resolves the default output file path.
    /// </summary>
    public Result TryParse()
    {
        if (_args.Count == 0)
            return Result.Fail("No arguments provided");

        if (!DescriptionEnum.TryParse<CommandType>(_args[0], ignoreCase: true, out var commandType))
            return Result.Fail($"Unknown command: {_args[0]}");

        CommandType = commandType;
        FilePath = Path.Combine(Directory.GetCurrentDirectory(), "output", "haversine.json");

        return Result.Ok();
    }

    /// <summary>
    /// Parses and validates --pairs and --seed flags. Populates <see cref="Pairs"/> and <see cref="Seed"/> on success.
    /// </summary>
    public Result TryParseGenerate()
    {
        if (_args.Count < 5)
            return Result.Fail("generate requires --pairs <count> --seed <value>");

        var pairs = 0;
        var seed = 0;

        for (var i = 0; i < _args.Count; i++)
        {
            var arg = _args[i];
            if (i + 1 >= _args.Count)
                return Result.Fail($"Missing value for argument: {arg}");

            var name = arg.Split("--").Last();
            var value = _args[i + 1];

            if (name == "pairs")
                pairs = int.Parse(value);

            if (name == "seed")
                seed = int.Parse(value);
        }

        if (pairs <= 0) return Result.Fail("Number of pairs must be greater than 0");
        if (seed <= 0) return Result.Fail("Seed must be non-negative");
        if (pairs > MaxPairs) return Result.Fail($"Number of pairs cannot exceed {MaxPairs:N0}");

        Pairs = pairs;
        Seed = seed;

        return Result.Ok();
    }
}
