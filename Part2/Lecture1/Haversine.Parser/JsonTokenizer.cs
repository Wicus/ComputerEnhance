using Haversine.Profiler;
using System.Globalization;

namespace Haversine.Parser;

public enum JsonTokenType
{
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Colon,          // :
    Comma,          // ,
    String,
    Number,
    True,
    False,
    Null,
    EndOfFile
}

public struct JsonToken
{
    public JsonTokenType Type;
    public string? StringValue;
    public double NumberValue;
}

public class JsonTokenizer(StreamReader reader, IProfiler profiler)
{
    private readonly StreamReader _reader = reader;
    private readonly IProfiler _profiler = profiler;
    private const int MaxStringLength = 4096;
    private const int MaxNumberLength = 64;

    public JsonToken NextToken()
    {
        using var zone = _profiler.BeginZone("Tokenizer.NextToken");
        SkipWhiteSpace();

        var peek = _reader.Peek();
        if (peek == -1)
        {
            return new JsonToken { Type = JsonTokenType.EndOfFile };
        }

        var ch = (char)peek;
        return ch switch
        {
            '{' => Consume(JsonTokenType.LeftBrace),
            '}' => Consume(JsonTokenType.RightBrace),
            '[' => Consume(JsonTokenType.LeftBracket),
            ']' => Consume(JsonTokenType.RightBracket),
            ':' => Consume(JsonTokenType.Colon),
            ',' => Consume(JsonTokenType.Comma),
            '"' => ReadString(),
            't' => ReadTrue(),
            'f' => ReadFalse(),
            'n' => ReadNull(),
            _ when IsNumericStart(ch) => ReadNumber(),
            _ => throw new InvalidDataException($"Unexpected character: '{ch}'")
        };
    }

    private JsonToken Consume(JsonTokenType type)
    {
        _reader.Read();
        return new JsonToken { Type = type };
    }

    private void SkipWhiteSpace()
    {
        while (true)
        {
            var peek = _reader.Peek();
            if (peek == -1 || !char.IsWhiteSpace((char)peek))
            {
                return;
            }
            _reader.Read();
        }
    }

    private JsonToken ReadString()
    {
        using var zone = _profiler.BeginZone("Tokenizer.ReadString");
        _reader.Read(); // consume opening "
        Span<char> buffer = stackalloc char[MaxStringLength];
        var length = 0;

        while (true)
        {
            var next = _reader.Read();
            if (next == -1)
            {
                throw new InvalidDataException("Unexpected end of file in string");
            }

            var ch = (char)next;
            if (ch == '"')
            {
                break;
            }

            if (ch == '\\')
            {
                next = _reader.Read();
                if (next == -1)
                {
                    throw new InvalidDataException("Unexpected end of file in string escape");
                }
                ch = (char)next switch
                {
                    '"' => '"',
                    '\\' => '\\',
                    '/' => '/',
                    'b' => '\b',
                    'f' => '\f',
                    'n' => '\n',
                    'r' => '\r',
                    't' => '\t',
                    _ => throw new InvalidDataException($"Invalid escape sequence: \\{(char)next}")
                };
            }

            if (length >= buffer.Length)
            {
                throw new InvalidDataException("String exceeds maximum length");
            }

            buffer[length++] = ch;
        }

        return new JsonToken
        {
            Type = JsonTokenType.String,
            StringValue = new string(buffer[..length])
        };
    }

    private JsonToken ReadNumber()
    {
        using var zone = _profiler.BeginZone("Tokenizer.ReadNumber");
        Span<char> buffer = stackalloc char[MaxNumberLength];
        var length = 0;

        while (true)
        {
            var peek = _reader.Peek();
            if (peek == -1 || !IsNumericChar(peek))
            {
                break;
            }

            if (length >= buffer.Length)
            {
                throw new InvalidDataException("Number exceeds maximum length");
            }

            buffer[length++] = (char)_reader.Read();
        }

        if (length == 0)
        {
            throw new InvalidDataException("Expected number");
        }

        var value = double.Parse(buffer[..length], NumberStyles.Float | NumberStyles.AllowLeadingSign, CultureInfo.InvariantCulture);

        return new JsonToken
        {
            Type = JsonTokenType.Number,
            NumberValue = value
        };
    }

    private JsonToken ReadTrue()
    {
        ExpectSequence("true");
        return new JsonToken { Type = JsonTokenType.True };
    }

    private JsonToken ReadFalse()
    {
        ExpectSequence("false");
        return new JsonToken { Type = JsonTokenType.False };
    }

    private JsonToken ReadNull()
    {
        ExpectSequence("null");
        return new JsonToken { Type = JsonTokenType.Null };
    }

    private void ExpectSequence(string expected)
    {
        foreach (var ch in expected)
        {
            var next = _reader.Read();
            if (next == -1 || (char)next != ch)
            {
                throw new InvalidDataException($"Expected '{expected}'");
            }
        }
    }

    private bool IsNumericStart(char ch)
    {
        return ch >= '0' && ch <= '9' || ch is '-';
    }

    private bool IsNumericChar(int ch)
    {
        return ch >= '0' && ch <= '9' || ch is '+' or '-' or '.' or 'e' or 'E';
    }
}
