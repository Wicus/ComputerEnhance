namespace Haversine.App;

readonly struct Result
{
    public bool IsSuccess { get; }
    public string Error { get; }

    private Result(bool isSuccess, string error)
    {
        IsSuccess = isSuccess;
        Error = error;
    }

    public static Result Ok() => new(true, string.Empty);
    public static Result Fail(string error) => new(false, error);

    public static implicit operator bool(Result r) => r.IsSuccess;
}

readonly struct Result<T>
{
    public bool IsSuccess { get; }
    public string Error { get; }

    private readonly T? _value;
    public T Value => IsSuccess ? _value! : throw new InvalidOperationException(Error);

    private Result(bool isSuccess, T? value, string error)
    {
        IsSuccess = isSuccess;
        _value = value;
        Error = error;
    }

    public static Result<T> Ok(T value) => new(true, value, string.Empty);
    public static Result<T> Fail(string error) => new(false, default, error);

    public static implicit operator bool(Result<T> r) => r.IsSuccess;
    public static implicit operator Result<T>(T value) => Ok(value);
}
