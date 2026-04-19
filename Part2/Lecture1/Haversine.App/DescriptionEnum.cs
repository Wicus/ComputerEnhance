using System.ComponentModel;
using System.Reflection;

namespace Haversine.App;

static class DescriptionEnum
{
    public static bool TryParse<T>(string value, bool ignoreCase, out T result) where T : struct, Enum
    {
        var comparison = ignoreCase ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;

        foreach (var field in typeof(T).GetFields(BindingFlags.Public | BindingFlags.Static))
        {
            var description = field.GetCustomAttribute<DescriptionAttribute>()?.Description;
            if (description != null && description == value.Equals(value, comparison))
            {
                result = (T)field.GetValue(null)!;
                return true;
            }
        }

        result = default;
        return false;
    }
}
