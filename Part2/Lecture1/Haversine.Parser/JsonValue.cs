namespace Haversine.Parser;

public enum JsonValueType
{
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
}

public class JsonValue
{
    public JsonValueType Type { get; }
    private readonly object? _value;

    private JsonValue(JsonValueType type, object? value = null)
    {
        Type = type;
        _value = value;
    }

    public static JsonValue Null() => new(JsonValueType.Null);
    public static JsonValue Boolean(bool value) => new(JsonValueType.Boolean, value);
    public static JsonValue Number(double value) => new(JsonValueType.Number, value);
    public static JsonValue String(string value) => new(JsonValueType.String, value);
    public static JsonValue Array(List<JsonValue> value) => new(JsonValueType.Array, value);
    public static JsonValue Object(Dictionary<string, JsonValue> value) => new(JsonValueType.Object, value);

    public bool AsBoolean() => Type == JsonValueType.Boolean ? (bool)_value! : throw new InvalidOperationException("Not a boolean");
    public double AsNumber() => Type == JsonValueType.Number ? (double)_value! : throw new InvalidOperationException("Not a number");
    public string AsString() => Type == JsonValueType.String ? (string)_value! : throw new InvalidOperationException("Not a string");
    public List<JsonValue> AsArray() => Type == JsonValueType.Array ? (List<JsonValue>)_value! : throw new InvalidOperationException("Not an array");
    public Dictionary<string, JsonValue> AsObject() => Type == JsonValueType.Object ? (Dictionary<string, JsonValue>)_value! : throw new InvalidOperationException("Not an object");

    public JsonValue this[string key] => AsObject()[key];
    public JsonValue this[int index] => AsArray()[index];
}
