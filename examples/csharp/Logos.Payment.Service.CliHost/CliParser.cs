namespace Logos.Payment.Service.CliHost;

public class CliParser
{
    private readonly string _command;
    private readonly List<string> _positionalArgs = new();
    private readonly Dictionary<string, string> _options = new();
    private readonly HashSet<string> _flags = new();

    public CliParser(string[] args)
    {
        if (args.Length < 1)
        {
            _command = string.Empty;
            return;
        }

        _command = args[0];

        for (int i = 1; i < args.Length; i++)
        {
            string arg = args[i];

            if (arg.StartsWith("--"))
            {
                string optionName = arg.Substring(2);

                // Check if next argument is a value (doesn't start with --)
                if (i + 1 < args.Length && !args[i + 1].StartsWith("--"))
                {
                    _options[optionName] = args[i + 1];
                    i++; // Skip the value
                }
                else
                {
                    // It's a flag
                    _flags.Add(optionName);
                }
            }
            else
            {
                _positionalArgs.Add(arg);
            }
        }
    }

    public string GetCommand() => _command;

    public bool HasFlag(string flag) => _flags.Contains(flag);

    public string GetOption(string option) => _options.TryGetValue(option, out var value) ? value : string.Empty;

    public string GetPositional(int index) => index < _positionalArgs.Count ? _positionalArgs[index] : string.Empty;

    public bool IsValid() => !string.IsNullOrEmpty(_command);
}
