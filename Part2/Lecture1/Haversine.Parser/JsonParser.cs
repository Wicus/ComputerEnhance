namespace Haversine.Parser;

public class JsonParser
{
    private readonly JsonTokenizer _tokenizer;
    private JsonToken _currentToken;

    public JsonParser(StreamReader reader)
    {
        _tokenizer = new JsonTokenizer(reader);
        _currentToken = _tokenizer.NextToken();
    }

    public JsonValue Parse()
    {
        var value = ParseValue();
        if (_currentToken.Type != JsonTokenType.EndOfFile)
        {
            throw new InvalidDataException("Expected end of file");
        }
        return value;
    }

    private JsonValue ParseValue()
    {
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
        var value = JsonValue.String(_currentToken.StringValue!);
        Advance();
        return value;
    }

    private JsonValue ParseNumber()
    {
        var value = JsonValue.Number(_currentToken.NumberValue);
        Advance();
        return value;
    }

    private JsonValue ParseTrue()
    {
        Advance();
        return JsonValue.Boolean(true);
    }

    private JsonValue ParseFalse()
    {
        Advance();
        return JsonValue.Boolean(false);
    }

    private JsonValue ParseNull()
    {
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
        _currentToken = _tokenizer.NextToken();
    }
}
