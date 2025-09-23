using System.CommandLine;
using System.CommandLine.Invocation;
using Haversine.Generator;
using Haversine.Parser;

var dataFile = "../output/data.json";

var rootCommand = new RootCommand("Haversine calculator - generates or parses coordinate data");

var parseCommand = new Command("parse", "Parse and calculate haversine distances from data file");
var generateCommand = new Command("generate", "Generate haversine coordinate pairs");
var pairsOption = new Option<int>("--pairs", "Number of coordinate pairs to generate") { IsRequired = true };
var seedOption = new Option<int>("--seed", "Random seed for generation") { IsRequired = true };

pairsOption.AddValidator(result =>
{
    var value = result.GetValueOrDefault<int>();
    if (value <= 0)
        return "Number of pairs must be greater than 0";
    else if (value > 10_000_000)
        return "Number of pairs cannot exceed 10,000,000";
    return null;
});

seedOption.AddValidator(result =>
{
    var value = result.GetValueOrDefault<int>();
    if (value < 0)
        return "Seed must be non-negative";
    return null;
});

generateCommand.AddOption(pairsOption);
generateCommand.AddOption(seedOption);
generateCommand.Handler = CommandHandler.Create<int, int>(
    (pairs, seed) =>
    {
        Generator.WriteJson(dataFile, pairs, seed);
    }
);

parseCommand.Handler = CommandHandler.Create(() => Parser.Parse(dataFile));

rootCommand.AddCommand(generateCommand);
rootCommand.AddCommand(parseCommand);

return await rootCommand.InvokeAsync(args);
