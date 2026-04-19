using System.ComponentModel;

namespace Haversine.App;

enum CommandType
{
    [Description("generate")]
    Generate,

    [Description("parse")]
    Parse,

    [Description("benchmark")]
    Benchmark,
}
