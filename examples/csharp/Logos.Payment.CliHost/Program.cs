using Microsoft.Extensions.DependencyInjection;
using Logos.Payment.CliHost;
using Logos.Payment.CliHost.Commands;
using Logos.Payment.CliHost.Configuration;
using Logos.Payment.Core.Application.UseCases;

// Parse command-line arguments
var parser = new CliParser(args);

if (!parser.IsValid() || parser.GetCommand() == "help")
{
    PrintHelp();
    return 0;
}

// Build service container
var services = new ServiceCollection();
ServiceConfiguration.ConfigureServices(services);
var serviceProvider = services.BuildServiceProvider();

// Route to command handlers
string command = parser.GetCommand();

try
{
    if (command == "authorize")
    {
        var useCase = serviceProvider.GetRequiredService<AuthorizePaymentUseCase>();
        var cmd = new AuthorizeCommand(useCase);
        return cmd.Execute(parser);
    }
    else if (command == "get")
    {
        var useCase = serviceProvider.GetRequiredService<GetPaymentUseCase>();
        var cmd = new GetPaymentCommand(useCase);
        return cmd.Execute(parser);
    }
    else
    {
        Console.Error.WriteLine($"Error: Unknown command: {command}\n");
        PrintHelp();
        return 1;
    }
}
finally
{
    if (serviceProvider is IDisposable disposable)
    {
        disposable.Dispose();
    }
}

static void PrintHelp()
{
    Console.WriteLine("Payment Service CLI\n");
    Console.WriteLine("Usage: payment-cli <command> [options]\n");
    Console.WriteLine("Commands:");
    Console.WriteLine("  authorize    Authorize a new payment");
    Console.WriteLine("  get          Retrieve payment details by ID");
    Console.WriteLine("  help         Show this help message\n");
    Console.WriteLine("Use 'payment-cli <command> --help' for more information about a command.\n");
}
