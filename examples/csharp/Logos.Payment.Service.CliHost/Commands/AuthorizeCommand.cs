using Logos.Payment.Service.Core.Application.UseCases;
using Logos.Payment.Service.Core.Application.Contracts;
using Logos.Payment.Service.Core.SharedKernel;

namespace Logos.Payment.Service.CliHost.Commands;

/// <summary>
/// CLI command (transport endpoint) for authorizing payments.
/// Parses CLI arguments and invokes AuthorizePaymentUseCase.
/// </summary>
public class AuthorizeCommand
{
    private readonly AuthorizePaymentUseCase _useCase;

    public AuthorizeCommand(AuthorizePaymentUseCase useCase)
    {
        _useCase = useCase;
    }

    public int Execute(CliParser parser)
    {
        if (parser.HasFlag("help"))
        {
            PrintUsage();
            return 0;
        }

        // Parse CLI arguments
        string amountStr = parser.GetOption("amount");
        string currency = parser.GetOption("currency");
        string merchantId = parser.GetOption("merchant");

        // Validate required arguments
        if (string.IsNullOrEmpty(amountStr) || string.IsNullOrEmpty(currency) || string.IsNullOrEmpty(merchantId))
        {
            Console.Error.WriteLine("Error: Missing required arguments\n");
            PrintUsage();
            return 1;
        }

        // Map CLI arguments to Contract Model
        try
        {
            if (!decimal.TryParse(amountStr, out decimal amount))
            {
                Console.Error.WriteLine($"Error: Invalid amount '{amountStr}'\n");
                return 1;
            }

            var request = new AuthorizePaymentRequest(
                Amount: new Money(amount, currency),
                MerchantId: merchantId);

            // Execute use case (synchronous)
            var result = _useCase.Execute(request);

            // Output result to console
            Console.WriteLine();
            Console.WriteLine("=== Payment Authorization Result ===");
            Console.WriteLine($"Payment ID:     {result.PaymentId}");
            Console.WriteLine($"Authorized:     {(result.IsAuthorized ? "YES" : "NO")}");
            Console.WriteLine($"Status:         {result.Status}");
            Console.WriteLine($"Amount:         {result.Amount}");

            if (result.DeclineReason != null)
            {
                Console.WriteLine($"Decline Reason: {result.DeclineReason}");
            }

            Console.WriteLine("====================================\n");

            return result.IsAuthorized ? 0 : 1;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
            return 1;
        }
    }

    public static void PrintUsage()
    {
        Console.WriteLine("Usage: payment-cli authorize [OPTIONS]\n");
        Console.WriteLine("Authorize a payment\n");
        Console.WriteLine("Options:");
        Console.WriteLine("  --amount AMOUNT       Payment amount (required)");
        Console.WriteLine("  --currency CURRENCY   Currency code, e.g., USD (required)");
        Console.WriteLine("  --merchant MERCHANT   Merchant identifier (required)");
        Console.WriteLine("  --help                Show this help message\n");
        Console.WriteLine("Example:");
        Console.WriteLine("  payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001\n");
    }
}
