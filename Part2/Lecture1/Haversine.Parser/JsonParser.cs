using Haversine.Profiler;

namespace Haversine.Parser;

public class JsonParser
{
    private readonly JsonTokenizer _tokenizer;
    private JsonToken _currentToken;
    private readonly IProfiler _profiler;

    public JsonParser(IProfiler profiler, StreamReader reader)
    {
        _tokenizer = new JsonTokenizer(reader, profiler);
        _currentToken = _tokenizer.NextToken();
        _profiler = profiler;
    }

    public JsonValue Parse()
    {
        using var parseZone = _profiler.BeginZone("Parse");
        var value = ParseValue();
        if (_currentToken.Type != JsonTokenType.EndOfFile)
        {
            throw new InvalidDataException("Expected end of file");
        }
        return value;
    }

    private JsonValue ParseValue()
    {
        using var parseObjectZone = _profiler.BeginZone("ParseValue");
        return _currentToken.Type switch
        {
            JsonTokenType.LeftBrace => ParseObject(),
            JsonTokenType.LeftBracket => ParseArray(),
            JsonTokenType.String => ParseString(),
            JsonTokenType.Number => ParseNumber(),
            JsonTokenType.True => ParseTrue(),
            JsonTokenType.False => ParseFalse(),
            JsonTokenType.Null => ParseNull(),
            _ => throw new InvalidDataException($"Unexpected token: {_currentToken.Type}")
        };
    }

    private JsonValue ParseObject()
    {
        using var zone = _profiler.BeginZone("ParseObject");
        Expect(JsonTokenType.LeftBrace);
        var dict = new Dictionary<string, JsonValue>();

        if (_currentToken.Type == JsonTokenType.RightBrace)
        {
            Advance();
            return JsonValue.Object(dict);
        }

        while (true)
        {
            if (_currentToken.Type != JsonTokenType.String)
            {
                throw new InvalidDataException("Expected string key in object");
            }

            var key = _currentToken.StringValue!;
            Advance();

            Expect(JsonTokenType.Colon);

            var value = ParseValue();
            dict[key] = value;

            if (_currentToken.Type == JsonTokenType.RightBrace)
            {
                Advance();
                break;
            }

            Expect(JsonTokenType.Comma);
        }

        return JsonValue.Object(dict);
    }

    private JsonValue ParseArray()
    {
        using var zone = _profiler.BeginZone("ParseArray");
        Expect(JsonTokenType.LeftBracket);
        var list = new List<JsonValue>();

        if (_currentToken.Type == JsonTokenType.RightBracket)
        {
            Advance();
            return JsonValue.Array(list);
        }

        while (true)
        {
            var value = ParseValue();
            list.Add(value);

            if (_currentToken.Type == JsonTokenType.RightBracket)
            {
                Advance();
                break;
            }

            Expect(JsonTokenType.Comma);
        }

        return JsonValue.Array(list);
    }

    private JsonValue ParseString()
    {
        using var zone = _profiler.BeginZone("ParseString");
        var value = JsonValue.String(_currentToken.StringValue!);
        Advance();
        return value;
    }

    private JsonValue ParseNumber()
    {
        using var zone = _profiler.BeginZone("ParseNumber");
        var value = JsonValue.Number(_currentToken.NumberValue);
        Advance();
        return value;
    }

    private JsonValue ParseTrue()
    {
        using var zone = _profiler.BeginZone("ParseTrue");
        Advance();
        return JsonValue.Boolean(true);
    }

    private JsonValue ParseFalse()
    {
        using var zone = _profiler.BeginZone("ParseFalse");
        Advance();
        return JsonValue.Boolean(false);
    }

    private JsonValue ParseNull()
    {
        using var zone = _profiler.BeginZone("ParseNull");
        Advance();
        return JsonValue.Null();
    }

    private void Expect(JsonTokenType expected)
    {
        if (_currentToken.Type != expected)
        {
            throw new InvalidDataException($"Expected {expected}, got {_currentToken.Type}");
        }
        Advance();
    }

    private void Advance()
    {
        using var zone = _profiler.BeginZone("Advance");
        _currentToken = _tokenizer.NextToken();
    }
}
