# CLI Host Example - C#

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C#](AP-002-Implementation-CSharp.md)

---

## 1. Introduction

This example demonstrates a command-line interface (CLI) host that invokes use cases from the [C# implementation example](AP-002-Implementation-CSharp.md).

> **Compilable Example Available**  
> A fully compilable version of this example is available at:  
> [`examples/csharp/MyPaymentService.CliHost/`](../../examples/csharp/MyPaymentService.CliHost/)
>
> You can build and run it with:
> ```bash
> cd examples/csharp
> dotnet build
> dotnet run --project MyPaymentService.CliHost -- help
> ```

**Key Points:**
- CLI parses command-line arguments
- Maps arguments to Contract Models
- Invokes Use Cases synchronously
- Outputs results to console with proper formatting
- Thread lifecycle: Parse -> Execute -> Output -> Exit

---

## 2. Project Structure

**Location:** `MyPaymentService.CliHost/`

```
MyPaymentService.Core/                    (From AP-002 example)
MyPaymentService.CliHost/
+-- Commands/
|   +-- AuthorizeCommand.cs
|   +-- GetPaymentCommand.cs
+-- Configuration/
|   +-- ServiceConfiguration.cs
+-- CliParser.cs
+-- XmlDocReader.cs
+-- Program.cs
```

The CLI Host is a thin adapter around the same Core (Application + Domain + Shared Kernel) used by the ASP.NET Core HTTP host. Only the transport layer differs.

---

## 3. CLI Parser

The CLI parser extracts commands, options, flags, and positional arguments from the command line.

**File:** `MyPaymentService.CliHost/CliParser.cs`

The `CliParser` class:
- Extracts command name (first argument)
- Parses flags (e.g., `--help`)
- Parses options with values (e.g., `--amount 100.00`)
- Collects positional arguments
- Provides type-safe accessors

```csharp
namespace MyPaymentService.CliHost;

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

    public string GetOption(string option) =>
        _options.TryGetValue(option, out var value) ? value : string.Empty;

    public string GetPositional(int index) =>
        index < _positionalArgs.Count ? _positionalArgs[index] : string.Empty;

    public bool IsValid() => !string.IsNullOrEmpty(_command);
}
```

---

## 4. Authorize Command

The authorize command handles payment authorization requests.

**File:** `MyPaymentService.CliHost/Commands/AuthorizeCommand.cs`

The `AuthorizeCommand` class:
- Parses CLI arguments (amount, currency, merchant)
- Creates `Money` from string input
- Maps to `AuthorizePaymentRequest` contract
- Invokes `AuthorizePaymentUseCase` synchronously
- Formats output for the console
- Returns exit code based on authorization result

```csharp
using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Application.Contracts;
using MyPaymentService.Core.SharedKernel;

namespace MyPaymentService.CliHost.Commands;

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

        // Map CLI arguments -> Contract Model
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
```

**Usage Example:**
```bash
payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001
```

**Output:**
```
=== Payment Authorization Result ===
Payment ID:     PAY-000001
Authorized:     YES
Status:         Authorized
Amount:         100.00 USD
====================================
```

---

## 5. Get Payment Command

The get payment command retrieves payment details by ID.

**File:** `MyPaymentService.CliHost/Commands/GetPaymentCommand.cs`

The `GetPaymentCommand` class:
- Extracts payment ID from positional argument
- Maps to `GetPaymentRequest` contract
- Invokes `GetPaymentUseCase`
- Formats payment details for display
- Handles not-found case

```csharp
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
```

**Usage Example:**
```bash
payment-cli get PAY-000001
```

**Output:**
```
=== Payment Details ===
Payment ID:     PAY-000001
Amount:         100.00 USD
Merchant ID:    MERCH-001
Status:         Authorized
Created At:     2024-12-07 10:30:45
=======================
```

---

## 6. Service Configuration

Dependency injection wiring is centralized so that the same Core services can be composed by any host.

**File:** `MyPaymentService.CliHost/Configuration/ServiceConfiguration.cs`

```csharp
using Microsoft.Extensions.DependencyInjection;
using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.Domain.Services;
using MyPaymentService.Core.SharedKernel;
using MyPaymentService.Infrastructure.InMemory;

namespace MyPaymentService.CliHost.Configuration;

public static class ServiceConfiguration
{
    public static void ConfigureServices(IServiceCollection services)
    {
        // Register infrastructure (capability implementations)
        services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
        services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

        // Register domain service with configuration
        services.AddSingleton<PaymentAuthorizationService>(sp =>
        {
            var fraudService = sp.GetRequiredService<IFraudDetectionService>();
            var maxAmount = new Money(10000m, "USD");
            return new PaymentAuthorizationService(fraudService, maxAmount);
        });

        // Register use cases
        services.AddScoped<AuthorizePaymentUseCase>();
        services.AddScoped<GetPaymentUseCase>();
    }
}
```

---

## 7. Main Entry Point

The `Program.cs` file wires everything together using top-level statements.

**File:** `MyPaymentService.CliHost/Program.cs`

The composition root:
1. **Parses command line** using `CliParser`
2. **Builds the service container** via `ServiceConfiguration`
3. **Resolves use cases** from the container
4. **Routes to command handlers** based on command name
5. **Executes command** and returns exit code
6. **Disposes the container** on exit

```csharp
using Microsoft.Extensions.DependencyInjection;
using MyPaymentService.CliHost;
using MyPaymentService.CliHost.Commands;
using MyPaymentService.CliHost.Configuration;
using MyPaymentService.Core.Application.Contracts;
using MyPaymentService.Core.Application.UseCases;

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
```

> **Note:** The compilable example enriches its `PrintHelp` output by reading XML documentation
> from the Contract Models via `XmlDocReader`, so command help stays in sync with the contracts.

---

## 8. Key Implementation Details

### 8.1 Money Formatting

The CLI relies on the `Money` value object's `ToString()` implementation for consistent,
type-safe formatting (amount plus currency code). There is no direct `decimal` exposure in output.

### 8.2 Synchronous Execution

The thread lifecycle is simple:
1. Parse arguments
2. Create request contract
3. Execute use case (synchronous)
4. Format and display result
5. Return exit code

No async/await, no background hosting - just straightforward sequential execution suited to a short-lived process.

### 8.3 Error Handling

- Invalid input: Shows usage message
- Payment not found: Returns error message and non-zero exit code
- Parsing errors: Caught and displayed with helpful context

---

## 9. Usage Examples

### 9.1 Authorize a Payment

```bash
$ payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001

=== Payment Authorization Result ===
Payment ID:     PAY-000001
Authorized:     YES
Status:         Authorized
Amount:         100.00 USD
====================================
```

### 9.2 Get Payment Details

```bash
$ payment-cli get PAY-000001

=== Payment Details ===
Payment ID:     PAY-000001
Amount:         100.00 USD
Merchant ID:    MERCH-001
Status:         Authorized
Created At:     2024-12-07 10:30:45
=======================
```

### 9.3 Authorization Declined

```bash
$ payment-cli authorize --amount 15000.00 --currency USD --merchant MERCH-001

=== Payment Authorization Result ===
Payment ID:     PAY-000002
Authorized:     NO
Status:         Declined
Amount:         15000.00 USD
Decline Reason: Amount exceeds maximum limit
====================================
```

### 9.4 Help

```bash
$ payment-cli help

Payment Service CLI

Usage: payment-cli <command> [options]

Commands:
  authorize    Authorize a new payment
  get          Retrieve payment details by ID
  help         Show this help message

Use 'payment-cli <command> --help' for more information about a command.
```

---

## 10. Key Principles

### 10.1 Thread Lifecycle

```
1. Parse command-line arguments
   |
2. Build service container
   |
3. Map CLI args -> Contract Model
   |
4. Invoke Use Case (synchronous)
   |
5. Format and output result
   |
6. Exit (return code)
```

The process is **short-lived**: parse -> execute -> output -> exit.

### 10.2 No Business Logic in CLI

The CLI Host:
- Parses command-line arguments
- Maps arguments to Contract Models
- Formats output for console
- Handles CLI-specific concerns (help text, exit codes)
- Does NOT make business decisions
- Does NOT contain business rules
- Does NOT orchestrate operations

All business logic is in the Use Case and Domain.

### 10.3 Command Pattern

Each CLI command:
- Takes a Use Case in its constructor (injected)
- Implements an `Execute(CliParser)` method
- Returns an exit code (0 = success, non-zero = error)
- Provides a `PrintUsage()` static method

### 10.4 Separation of Concerns

- **CLI Arguments** - Command-line format (`--amount 100`)
- **Contract Models** - Application boundary
- **Value Objects** - Shared domain concepts

The command maps between CLI arguments and Contract Models.

---

## 11. Comparison with ASP.NET Core Web API

| Aspect | C# CLI Host | C# Web API Host |
|--------|-------------|-----------------|
| **Entry Point** | Command line | HTTP endpoint |
| **Request Format** | CLI arguments | JSON body |
| **Response Format** | Console output | JSON response |
| **Thread Model** | Parse -> Execute -> Exit | HTTP request -> Execute -> HTTP response |
| **DI Container** | `ServiceCollection` (manual) | Built-in host DI |
| **Use Case Invocation** | Same `AuthorizePaymentUseCase` | Same `AuthorizePaymentUseCase` |
| **Domain Logic** | Identical | Identical |

Both are thin adapters around the same use cases and domain logic.

---

## 12. Summary

This example demonstrates:

1. **CLI argument parsing** - Simple but effective parser
2. **Command pattern** - Each command is a separate class
3. **Dependency injection** - Same Core wiring as other hosts
4. **Use Case invocation** - Direct, synchronous calls
5. **Thread lifecycle** - Short-lived: parse -> execute -> output -> exit
6. **Separation**: CLI args -> Contract Models -> Value Objects
7. **No business logic** - CLI only parses, invokes, and formats

The pattern is simple: parse arguments -> map to contract -> invoke use case -> format output -> exit.

---

**See Also:**
- [C# Implementation (AP-002)](AP-002-Implementation-CSharp.md) - Base implementation this builds on
- [CLI Host Example - C++](AP-003-CLI-Cpp.md) - C++ equivalent
- AP-003 - Incoming Implementations (specification)

**End of Document**
