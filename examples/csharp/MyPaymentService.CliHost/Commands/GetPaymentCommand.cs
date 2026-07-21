using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Application.Contracts;

namespace MyPaymentService.CliHost.Commands;

/// <summary>
/// CLI command (transport endpoint) for retrieving payment details.
/// Parses CLI arguments and invokes GetPaymentUseCase.
/// </summary>
public class GetPaymentCommand
{
    private readonly GetPaymentUseCase _useCase;

    public GetPaymentCommand(GetPaymentUseCase useCase)
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

        // Get payment ID from positional argument
        string paymentId = parser.GetPositional(0);

        if (string.IsNullOrEmpty(paymentId))
        {
            Console.Error.WriteLine("Error: Missing payment ID\n");
            PrintUsage();
            return 1;
        }

        // Execute use case (synchronous)
        var result = _useCase.Execute(new GetPaymentRequest(paymentId));

        if (result == null)
        {
            Console.Error.WriteLine($"Error: Payment not found: {paymentId}");
            return 1;
        }

        // Output result to console
        Console.WriteLine();
        Console.WriteLine("=== Payment Details ===");
        Console.WriteLine($"Payment ID:     {result.PaymentId}");
        Console.WriteLine($"Amount:         {result.Amount}");
        Console.WriteLine($"Merchant ID:    {result.MerchantId}");
        Console.WriteLine($"Status:         {result.Status}");
        Console.WriteLine($"Created At:     {result.CreatedAt:yyyy-MM-dd HH:mm:ss}");

        if (result.DeclineReason != null)
        {
            Console.WriteLine($"Decline Reason: {result.DeclineReason}");
        }

        Console.WriteLine("=======================\n");

        return 0;
    }

    public static void PrintUsage()
    {
        Console.WriteLine("Usage: payment-cli get <payment-id> [OPTIONS]\n");
        Console.WriteLine("Retrieve payment details by ID\n");
        Console.WriteLine("Arguments:");
        Console.WriteLine("  payment-id            Payment identifier (required)\n");
        Console.WriteLine("Options:");
        Console.WriteLine("  --help                Show this help message\n");
        Console.WriteLine("Example:");
        Console.WriteLine("  payment-cli get PAY-000001\n");
    }
}
