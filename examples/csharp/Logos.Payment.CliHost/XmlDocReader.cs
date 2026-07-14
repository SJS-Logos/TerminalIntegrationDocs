using System.Reflection;
using System.Xml.Linq;

namespace Logos.Payment.CliHost;

/// <summary>
/// Reads XML documentation from assemblies at runtime.
/// </summary>
public static class XmlDocReader
{
    private static XDocument? _xmlDoc;

    /// <summary>
    /// Gets the summary documentation for a type.
    /// </summary>
    public static string? GetTypeSummary<T>()
    {
        return GetTypeSummary(typeof(T));
    }

    /// <summary>
    /// Gets the summary documentation for a type.
    /// </summary>
    public static string? GetTypeSummary(Type type)
    {
        var doc = LoadXmlDoc(type.Assembly);
        if (doc == null) return null;

        var memberName = $"T:{type.FullName}";
        return GetSummary(doc, memberName);
    }

    /// <summary>
    /// Gets the summary documentation for a property or parameter.
    /// </summary>
    public static string? GetParameterSummary<T>(string parameterName)
    {
        return GetParameterSummary(typeof(T), parameterName);
    }

    /// <summary>
    /// Gets the summary documentation for a record constructor parameter.
    /// </summary>
    public static string? GetParameterSummary(Type type, string parameterName)
    {
        var doc = LoadXmlDoc(type.Assembly);
        if (doc == null) return null;

        // For positional records, parameters are documented with <param> tags on the type
        var memberName = $"T:{type.FullName}";
        return GetParamDoc(doc, memberName, parameterName);
    }

    private static XDocument? LoadXmlDoc(Assembly assembly)
    {
        if (_xmlDoc != null) return _xmlDoc;

        var xmlPath = Path.ChangeExtension(assembly.Location, ".xml");
        if (!File.Exists(xmlPath)) return null;

        _xmlDoc = XDocument.Load(xmlPath);
        return _xmlDoc;
    }

    private static string? GetSummary(XDocument doc, string memberName)
    {
        var member = doc.Descendants("member")
            .FirstOrDefault(m => m.Attribute("name")?.Value == memberName);

        var summary = member?.Element("summary")?.Value;
        return summary?.Trim().Replace("\n", " ").Replace("  ", " ");
    }

    private static string? GetParamDoc(XDocument doc, string memberName, string parameterName)
    {
        var member = doc.Descendants("member")
            .FirstOrDefault(m => m.Attribute("name")?.Value == memberName);

        var param = member?.Elements("param")
            .FirstOrDefault(p => p.Attribute("name")?.Value == parameterName);

        return param?.Value?.Trim().Replace("\n", " ").Replace("  ", " ");
    }
}
