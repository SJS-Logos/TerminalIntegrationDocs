using Microsoft.Extensions.DependencyInjection;
using Logos.Payment.CliHost;
using Logos.Payment.CliHost.Commands;
using Logos.Payment.CliHost.Configuration;
using Logos.Payment.Core.Application.Contracts;
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

    // Authorize command - documentation from AuthorizePaymentRequest
    Console.WriteLine("  authorize    Authorize a new payment");
    var authSummary = XmlDocReader.GetTypeSummary<AuthorizePaymentRequest>();
    if (!string.IsNullOrEmpty(authSummary))
        Console.WriteLine($"               {authSummary}");
    Console.WriteLine("               Parameters:");
    var amountDoc = XmlDocReader.GetParameterSummary<AuthorizePaymentRequest>("Amount");
    var merchantDoc = XmlDocReader.GetParameterSummary<AuthorizePaymentRequest>("MerchantId");
    Console.WriteLine($"                 --amount      {amountDoc ?? "The payment amount"}");
    Console.WriteLine($"                 --merchant-id {merchantDoc ?? "The merchant identifier"}");
    Console.WriteLine();

    // Get command - documentation from GetPaymentRequest
    Console.WriteLine("  get          Retrieve payment details by ID");
    var getSummary = XmlDocReader.GetTypeSummary<GetPaymentRequest>();
    if (!string.IsNullOrEmpty(getSummary))
        Console.WriteLine($"               {getSummary}");
    Console.WriteLine("               Parameters:");
    var paymentIdDoc = XmlDocReader.GetParameterSummary<GetPaymentRequest>("PaymentId");
    Console.WriteLine($"                 --payment-id  {paymentIdDoc ?? "The payment identifier"}");
    Console.WriteLine();

    Console.WriteLine("  help         Show this help message\n");
    Console.WriteLine("Use 'payment-cli <command> --help' for more information about a command.\n");
}
