# Logos Payment CLI Host

A command-line interface (CLI) for the Logos Payment Service, demonstrating the AP-003 Host Unit pattern.

## Structure

```
Logos.Payment.CliHost/
??? Commands/                    # Transport endpoints (CLI commands)
?   ??? AuthorizeCommand.cs
?   ??? GetPaymentCommand.cs
??? Configuration/
?   ??? ServiceConfiguration.cs  # Dependency injection setup
??? CliParser.cs                 # CLI argument parser
??? Program.cs                   # Composition root
```

## Building

```bash
cd examples/csharp
dotnet build Logos.Payment.CliHost
```

## Running

### Authorize a Payment

```bash
dotnet run --project Logos.Payment.CliHost -- authorize --amount 100.00 --currency USD --merchant MERCH-001
```

Or use the compiled executable:

```bash
cd Logos.Payment.CliHost/bin/Debug/net8.0
.\Logos.Payment.CliHost.exe authorize --amount 100.00 --currency USD --merchant MERCH-001
```

Output:
```
=== Payment Authorization Result ===
Payment ID:     d0f55e76-7159-4cb5-8ab6-66526ca6cdf1
Authorized:     YES
Status:         Authorized
Amount:         100.00 USD
====================================
```

### Get Payment Details

```bash
dotnet run --project Logos.Payment.CliHost -- get <payment-id>
```

Example:
```bash
.\Logos.Payment.CliHost.exe get d0f55e76-7159-4cb5-8ab6-66526ca6cdf1
```

**Note**: The in-memory repository does not persist across CLI invocations. Each run creates a fresh instance.

### Help

```bash
dotnet run --project Logos.Payment.CliHost -- help
dotnet run --project Logos.Payment.CliHost -- authorize --help
dotnet run --project Logos.Payment.CliHost -- get --help
```

## Architecture

This CLI follows AP-003 (Incoming Implementations):

- **Commands/** - Transport endpoints that parse CLI arguments and invoke use cases
- **Configuration/** - Service registration and dependency injection
- **Program.cs** - Composition root that wires commands to the service container

The CLI depends on:
- `Logos.Payment.Core` - Domain, SharedKernel, Capabilities, and Application
- `Logos.Payment.Infrastructure.InMemory` - In-memory implementations of capabilities

Business logic remains in the Core; the CLI is a thin translation layer from command-line arguments to use case contracts.

## Examples

**Successful authorization (amount < $5,000):**
```bash
.\Logos.Payment.CliHost.exe authorize --amount 49.99 --currency USD --merchant MERCH-001
# Exit code: 0
```

**Declined (fraud detection threshold > $5,000):**
```bash
.\Logos.Payment.CliHost.exe authorize --amount 5001.00 --currency USD --merchant MERCH-001
# Exit code: 1
# Decline Reason: Flagged by fraud detection
```

**Declined (exceeds maximum $10,000):**
```bash
.\Logos.Payment.CliHost.exe authorize --amount 15000.00 --currency USD --merchant MERCH-001
# Exit code: 1
# Decline Reason: Amount exceeds maximum
```
