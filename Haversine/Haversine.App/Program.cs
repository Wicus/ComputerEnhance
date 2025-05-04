using Haversine.Generator;
using Haversine.Parser;

var parsedArgs = ParseArgs(args);
var dataFile = "../output/data.json";

switch (parsedArgs.Type)
{
    case Type.Generate:
        Generator.WriteJson(dataFile, parsedArgs.Pairs!.Value, parsedArgs.Seed!.Value);
        break;
    case Type.Parse:
        Parser.Parse(dataFile);
        break;
    default:
        throw new ArgumentException("Invalid type");
}

static Args ParseArgs(string[] args)
{
    if (!Enum.TryParse<Type>(args[0], out var type))
    {
        throw new ArgumentException("Invalid type");
    }

    if (type == Type.Generate)
    {
        if (!int.TryParse(args[1], out var pairs))
        {
            throw new ArgumentException("Invalid pairs");
        }
        if (!int.TryParse(args[2], out var seed))
        {
            throw new ArgumentException("Invalid seed");
        }
        if (pairs <= 0)
        {
            throw new ArgumentException("Invalid number of pairs");
        }
        if (pairs > 10_000_000)
        {
            throw new ArgumentException("Number of pairs too large");
        }
        if (seed < 0)
        {
            throw new ArgumentException("Invalid seed");
        }
        return new Args
        {
            Type = type,
            Pairs = pairs,
            Seed = seed,
        };
    }

    return new Args
    {
        Type = type,
    };
}

enum Type
{
    Unknown,
    Generate,
    Parse,
}

class Args
{
    public Type Type { get; set; } = Type.Unknown;
    public int? Pairs { get; set; }
    public int? Seed { get; set; }
}
