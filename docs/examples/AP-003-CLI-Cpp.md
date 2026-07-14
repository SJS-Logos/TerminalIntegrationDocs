# CLI Application Example: C++

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C++](AP-002-Implementation-Cpp.md)

---

## 1. Introduction

This example demonstrates a command-line interface (CLI) that invokes use cases from the [C++ implementation example](AP-002-Implementation-Cpp.md).

> **?? Compilable Example Available**  
> A fully compilable version of this example is available at:  
> [`examples/cpp/`](../../examples/cpp/)  
>
> You can build and run it with:
> ```bash
> cd examples/cpp
> ./build.sh           # Linux/macOS
> build.bat            # Windows
> ./payment_cli help   # Run the CLI
> ```

**Key Points:**
- CLI parses command-line arguments
- Maps arguments to Contract Models
- Invokes Use Cases synchronously
- Outputs results to console with proper formatting
- Thread lifecycle: Parse ? Execute ? Output ? Exit

---

## 2. Project Structure

**Location:** `logos_payment_cli_host/`

```
logos_payment_core/                    (From AP-002 example)
logos_payment_cli_host/
??? commands/
?   ??? authorize_command.h/.cpp
?   ??? get_payment_command.h/.cpp
??? mappings/
?   ??? cli_argument_mapping.h/.cpp
??? configuration/
?   ??? cli_configuration.h/.cpp
??? cli_parser.h/.cpp
??? main.cpp
```

---

## 3. CLI Parser

The CLI parser extracts commands, options, flags, and positional arguments from the command line.

**Header:** `logos_payment_cli_host/cli_parser.h`

**Implementation:** `logos_payment_cli_host/cli_parser.cpp`

The `CliParser` class:
- Extracts command name (first argument)
- Parses flags (e.g., `--help`)
- Parses options with values (e.g., `--amount 100.00`)
- Collects positional arguments
- Provides type-safe accessors

---

## 4. Authorize Command

The authorize command handles payment authorization requests.

**Header:** `logos_payment_cli_host/commands/authorize_command.h`

**Implementation:** `logos_payment_cli_host/commands/authorize_command.cpp`

The `AuthorizeCommand` class:
- Parses CLI arguments (amount, currency, merchant)
- Creates `Money` from string input
- Maps to `AuthorizePaymentRequest` contract
- Invokes `AuthorizePaymentUseCase` synchronously
- Formats output using `Money::ToString()` for proper display
- Returns exit code based on authorization result

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

**Header:** `logos_payment_cli_host/commands/get_payment_command.h`

**Implementation:** `logos_payment_cli_host/commands/get_payment_command.cpp`

The `GetPaymentCommand` class:
- Extracts payment ID from positional argument
- Invokes `GetPaymentUseCase`
- Formats payment details for display
- Uses `Money::ToString()` for amount formatting
- Handles not-found case

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

## 6. Main Entry Point

The main function wires everything together.

**File:** `logos_payment_cli_host/main.cpp`

The composition root:
1. **Parses command line** using `CliParser`
2. **Loads configuration**
3. **Creates service container** for dependency injection
4. **Registers domain services** with configuration
5. **Creates use cases** with resolved dependencies
6. **Routes to command handlers** based on command name
7. **Executes command** and returns exit code

---

## 7. Key Implementation Details

### Money Formatting

The CLI uses `Money::ToString()` which provides:
- Automatic thousands separators (e.g., "1,234.56")
- Currency code display
- No double exposure - type-safe formatting

Example:
```cpp
Money amount = Money::FromCents(123456, "USD");
std::cout << amount.ToString() << "\n";  // Output: "1,234.56 USD"
```

### Synchronous Execution

The thread lifecycle is simple:
1. Parse arguments
2. Create request contract
3. Execute use case (synchronous)
4. Format and display result
5. Exit process

No async/await, no callbacks, no futures - just straightforward sequential execution.

### Error Handling

- Invalid input: Shows usage message
- Payment not found: Returns error message and non-zero exit code
- Parsing errors: Caught and displayed with helpful context

---

## 8. Running the Example

See [QUICKSTART.md](../../examples/cpp/QUICKSTART.md) for detailed instructions.

### Build
```bash
cd examples/cpp
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run Examples

```bash
# Show help
./payment_cli help

# Authorize a payment
./payment_cli authorize --amount 100.00 --currency USD --merchant MERCH-001

# Test fraud detection (> $5,000)
./payment_cli authorize --amount 5000.01 --currency USD --merchant MERCH-001

# Get payment details
./payment_cli get PAY-000001
```

---

## 9. Comparison with C# Web API

| Aspect | C++ CLI | C# Web API |
|--------|---------|------------|
| **Entry Point** | Command line | HTTP endpoint |
| **Request Format** | CLI arguments | JSON body |
| **Response Format** | Console output | JSON response |
| **Thread Model** | Parse ? Execute ? Exit | HTTP request ? Execute ? HTTP response |
| **Use Case Invocation** | Same `AuthorizePaymentUseCase` | Same `AuthorizePaymentUseCase` |
| **Domain Logic** | Identical | Identical |

Both are thin adapters around the same use cases and domain logic.

---

**End of Document**

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.substr(0, 2) == "--") {
            // Option with value: --amount 100
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options_[arg.substr(2)] = argv[i + 1];
                ++i;
            } else {
                // Flag: --help
                flags_.push_back(arg.substr(2));
            }
        } else {
            // Positional argument
            positional_args_.push_back(arg);
        }
    }
}

std::string CliParser::GetCommand() const {
    return command_;
}

bool CliParser::HasFlag(const std::string& flag) const {
    return std::find(flags_.begin(), flags_.end(), flag) != flags_.end();
}

std::string CliParser::GetOption(const std::string& option) const {
    auto it = options_.find(option);
    return it != options_.end() ? it->second : "";
}

std::string CliParser::GetPositional(size_t index) const {
    return index < positional_args_.size() ? positional_args_[index] : "";
}

bool CliParser::IsValid() const {
    return !command_.empty();
}

} // namespace logos::payment_service::cli
```

---

## 4. Authorize Command

### 4.1 Command Header

```cpp
// logos_payment_service_cli/commands/authorize_command.h
#pragma once

#include "application/use_cases/authorize_payment_use_case.h"
#include "cli_parser.h"

namespace logos::payment_service::cli::commands {

/// @brief CLI command for authorizing payments
/// @details Parses CLI arguments and invokes AuthorizePaymentUseCase
/// 
/// Usage:
///   payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001
/// 
/// @see application::use_cases::AuthorizePaymentUseCase
class AuthorizeCommand {
public:
    explicit AuthorizeCommand(
        application::use_cases::AuthorizePaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    application::use_cases::AuthorizePaymentUseCase* use_case_;  // Non-owning
};

} // namespace logos::payment_service::cli::commands
```

### 4.2 Command Implementation

```cpp
// logos_payment_service_cli/commands/authorize_command.cpp
#include "authorize_command.h"
#include "domain/value_objects/money.h"
#include <iostream>
#include <iomanip>

namespace logos::payment_service::cli::commands {

AuthorizeCommand::AuthorizeCommand(
    application::use_cases::AuthorizePaymentUseCase* use_case)
    : use_case_(use_case) {
}

int AuthorizeCommand::Execute(const CliParser& parser) {
    if (parser.HasFlag("help")) {
        PrintUsage();
        return 0;
    }

    // Parse CLI arguments
    std::string amount_str = parser.GetOption("amount");
    std::string currency = parser.GetOption("currency");
    std::string merchant_id = parser.GetOption("merchant");

    // Validate required arguments
    if (amount_str.empty() || currency.empty() || merchant_id.empty()) {
        std::cerr << "Error: Missing required arguments\n\n";
        PrintUsage();
        return 1;
    }

    double amount;
    try {
        amount = std::stod(amount_str);
    } catch (...) {
        std::cerr << "Error: Invalid amount: " << amount_str << "\n";
        return 1;
    }

    // Map CLI arguments ? Contract Model
    application::contracts::AuthorizePaymentRequest request{
        domain::value_objects::Money(amount, currency),
        merchant_id
    };

    // Execute use case (synchronous)
    auto result = use_case_->Execute(request);

    // Output result to console
    std::cout << "\n";
    std::cout << "=== Payment Authorization Result ===\n";
    std::cout << "Payment ID:     " << result.id << "\n";
    std::cout << "Authorized:     " << (result.is_authorized ? "YES" : "NO") << "\n";
    std::cout << "Status:         " << PaymentStatusToString(result.status) << "\n";
    std::cout << "Amount:         " << std::fixed << std::setprecision(2) 
              << result.amount.GetAmount() << " " << result.amount.GetCurrency() << "\n";

    if (result.decline_reason) {
        std::cout << "Decline Reason: " << *result.decline_reason << "\n";
    }

    std::cout << "====================================\n\n";

    return result.is_authorized ? 0 : 1;
}

void AuthorizeCommand::PrintUsage() {
    std::cout << "Usage: payment-cli authorize [OPTIONS]\n\n";
    std::cout << "Authorize a payment\n\n";
    std::cout << "Options:\n";
    std::cout << "  --amount AMOUNT       Payment amount (required)\n";
    std::cout << "  --currency CURRENCY   Currency code, e.g., USD (required)\n";
    std::cout << "  --merchant MERCHANT   Merchant identifier (required)\n";
    std::cout << "  --help                Show this help message\n\n";
    std::cout << "Example:\n";
    std::cout << "  payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001\n\n";
}

std::string AuthorizeCommand::PaymentStatusToString(
    domain::value_objects::PaymentStatus status) {
    switch (status) {
        case domain::value_objects::PaymentStatus::Pending:
            return "Pending";
        case domain::value_objects::PaymentStatus::Authorized:
            return "Authorized";
        case domain::value_objects::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment_service::cli::commands
```

---

## 5. Get Payment Command

### 5.1 Command Header

```cpp
// logos_payment_service_cli/commands/get_payment_command.h
#pragma once

#include "application/use_cases/get_payment_use_case.h"
#include "cli_parser.h"

namespace logos::payment_service::cli::commands {

/// @brief CLI command for retrieving payment details
/// @details Parses CLI arguments and invokes GetPaymentUseCase
/// 
/// Usage:
///   payment-cli get <payment-id>
/// 
/// @see application::use_cases::GetPaymentUseCase
class GetPaymentCommand {
public:
    explicit GetPaymentCommand(application::use_cases::GetPaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    application::use_cases::GetPaymentUseCase* use_case_;  // Non-owning
};

} // namespace logos::payment_service::cli::commands
```

### 5.2 Command Implementation

```cpp
// logos_payment_service_cli/commands/get_payment_command.cpp
#include "get_payment_command.h"
#include <iostream>
#include <iomanip>
#include <ctime>

namespace logos::payment_service::cli::commands {

GetPaymentCommand::GetPaymentCommand(
    application::use_cases::GetPaymentUseCase* use_case)
    : use_case_(use_case) {
}

int GetPaymentCommand::Execute(const CliParser& parser) {
    if (parser.HasFlag("help")) {
        PrintUsage();
        return 0;
    }

    // Get payment ID from positional argument
    std::string payment_id = parser.GetPositional(0);

    if (payment_id.empty()) {
        std::cerr << "Error: Missing payment ID\n\n";
        PrintUsage();
        return 1;
    }

    // Execute use case (synchronous)
    auto result = use_case_->Execute(payment_id);

    if (!result) {
        std::cerr << "Error: Payment not found: " << payment_id << "\n";
        return 1;
    }

    // Output result to console
    std::cout << "\n";
    std::cout << "=== Payment Details ===\n";
    std::cout << "Payment ID:     " << result->id << "\n";
    std::cout << "Amount:         " << std::fixed << std::setprecision(2)
              << result->amount.GetAmount() << " " << result->amount.GetCurrency() << "\n";
    std::cout << "Merchant ID:    " << result->merchant_id << "\n";
    std::cout << "Status:         " << PaymentStatusToString(result->status) << "\n";

    // Format timestamp
    auto time_t = std::chrono::system_clock::to_time_t(result->created_at);
    std::cout << "Created At:     " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";

    if (result->decline_reason) {
        std::cout << "Decline Reason: " << *result->decline_reason << "\n";
    }

    std::cout << "=======================\n\n";

    return 0;
}

void GetPaymentCommand::PrintUsage() {
    std::cout << "Usage: payment-cli get <payment-id> [OPTIONS]\n\n";
    std::cout << "Retrieve payment details by ID\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  payment-id            Payment identifier (required)\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help                Show this help message\n\n";
    std::cout << "Example:\n";
    std::cout << "  payment-cli get 550e8400-e29b-41d4-a716-446655440000\n\n";
}

std::string GetPaymentCommand::PaymentStatusToString(
    domain::value_objects::PaymentStatus status) {
    switch (status) {
        case domain::value_objects::PaymentStatus::Pending:
            return "Pending";
        case domain::value_objects::PaymentStatus::Authorized:
            return "Authorized";
        case domain::value_objects::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment_service::cli::commands
```

---

## 6. Main CLI Entry Point

```cpp
// logos_payment_service_cli/main.cpp
#include <iostream>
#include <memory>
#include "cli_parser.h"
#include "commands/authorize_command.h"
#include "commands/get_payment_command.h"
#include "application/container/service_container.h"
#include "domain/value_objects/money.h"
#include "domain/services/payment_authorization_service.h"
#include "adapters/sqlite_payment_repository.h"
#include "adapters/fraud_detection_service.h"

using namespace logos::payment_service;

void PrintUsage() {
    std::cout << "Usage: payment-cli <command> [OPTIONS]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  authorize     Authorize a payment\n";
    std::cout << "  get           Retrieve payment details\n";
    std::cout << "  help          Show this help message\n\n";
    std::cout << "Run 'payment-cli <command> --help' for command-specific help\n\n";
}

int main(int argc, char* argv[]) {
    cli::CliParser parser(argc, argv);

    if (!parser.IsValid() || parser.GetCommand() == "help") {
        PrintUsage();
        return parser.IsValid() ? 0 : 1;
    }

    try {
        // Load configuration (simplified - use proper config library)
        auto maximum_amount = domain::value_objects::Money(10000.0, "USD");

        // Create service container
        application::container::ServiceContainer container;

        // Register capability implementations (container takes ownership)
        container.Register<domain::abstractions::IPaymentRepository>(
            std::make_unique<adapters::SqlitePaymentRepository>("payments.db"));

        container.Register<domain::abstractions::IFraudDetectionService>(
            std::make_unique<adapters::ThirdPartyFraudDetectionService>(
                "https://fraud-api.example.com", "api-key-here"));

        // Register domain services
        container.Register<domain::services::PaymentAuthorizationService>(
            std::make_unique<domain::services::PaymentAuthorizationService>(
                container.Resolve<domain::abstractions::IFraudDetectionService>(),
                maximum_amount));

        // Register use cases
        container.Register<application::use_cases::AuthorizePaymentUseCase>(
            std::make_unique<application::use_cases::AuthorizePaymentUseCase>(
                container.Resolve<domain::services::PaymentAuthorizationService>(),
                container.Resolve<domain::abstractions::IPaymentRepository>()));

        container.Register<application::use_cases::GetPaymentUseCase>(
            std::make_unique<application::use_cases::GetPaymentUseCase>(
                container.Resolve<domain::abstractions::IPaymentRepository>()));

        // Route to appropriate command
        std::string command = parser.GetCommand();

        if (command == "authorize") {
            cli::commands::AuthorizeCommand cmd(
                container.Resolve<application::use_cases::AuthorizePaymentUseCase>());
            return cmd.Execute(parser);
        } else if (command == "get") {
            cli::commands::GetPaymentCommand cmd(
                container.Resolve<application::use_cases::GetPaymentUseCase>());
            return cmd.Execute(parser);
        } else {
            std::cerr << "Error: Unknown command: " << command << "\n\n";
            PrintUsage();
            return 1;
        }

    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
```

---

## 7. CMakeLists.txt

```cmake
# CLI application
add_executable(logos_payment_service_cli
    main.cpp
    cli_parser.cpp
    commands/authorize_command.cpp
    commands/get_payment_command.cpp
)

target_link_libraries(logos_payment_service_cli
    logos_payment_service_application
    logos_payment_service_domain
    logos_payment_service_adapters
)

target_include_directories(logos_payment_service_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install
install(TARGETS logos_payment_service_cli
    RUNTIME DESTINATION bin
    RENAME payment-cli
)
```

---

## 8. Usage Examples

### 8.1 Authorize a Payment

```bash
$ payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001

=== Payment Authorization Result ===
Payment ID:     550e8400-e29b-41d4-a716-446655440000
Authorized:     YES
Status:         Authorized
Amount:         100.00 USD
====================================
```

### 8.2 Get Payment Details

```bash
$ payment-cli get 550e8400-e29b-41d4-a716-446655440000

=== Payment Details ===
Payment ID:     550e8400-e29b-41d4-a716-446655440000
Amount:         100.00 USD
Merchant ID:    MERCH-001
Status:         Authorized
Created At:     2024-01-15 10:30:45
=======================
```

### 8.3 Authorization Declined

```bash
$ payment-cli authorize --amount 15000.00 --currency USD --merchant MERCH-001

=== Payment Authorization Result ===
Payment ID:     650e8400-e29b-41d4-a716-446655440001
Authorized:     NO
Status:         Declined
Amount:         15000.00 USD
Decline Reason: Amount exceeds maximum limit
====================================
```

### 8.4 Help

```bash
$ payment-cli help

Usage: payment-cli <command> [OPTIONS]

Commands:
  authorize     Authorize a payment
  get           Retrieve payment details
  help          Show this help message

Run 'payment-cli <command> --help' for command-specific help
```

```bash
$ payment-cli authorize --help

Usage: payment-cli authorize [OPTIONS]

Authorize a payment

Options:
  --amount AMOUNT       Payment amount (required)
  --currency CURRENCY   Currency code, e.g., USD (required)
  --merchant MERCHANT   Merchant identifier (required)
  --help                Show this help message

Example:
  payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001
```

---

## 9. Key Principles

### 9.1 Thread Lifecycle

```
1. Parse command-line arguments
   ?
2. Setup service container
   ?
3. Map CLI args ? Contract Model
   ?
4. Invoke Use Case (synchronous)
   ?
5. Format and output result
   ?
6. Exit (return code)
```

The process is **short-lived**: parse ? execute ? output ? exit.

### 9.2 No Business Logic in CLI

The CLI layer:
- ? Parses command-line arguments
- ? Maps arguments to Contract Models
- ? Formats output for console
- ? Handles CLI-specific concerns (help text, exit codes)
- ? Does NOT make business decisions
- ? Does NOT contain business rules
- ? Does NOT orchestrate operations

All business logic is in the Use Case and Domain.

### 9.3 Command Pattern

Each CLI command:
- Takes a Use Case in constructor (non-owning pointer)
- Implements `Execute(CliParser)` method
- Returns exit code (0 = success, non-zero = error)
- Provides `PrintUsage()` static method

### 9.4 Separation of Concerns

- **CLI Arguments** - Command-line format (`--amount 100`)
- **Contract Models** - Application boundary
- **Value Objects** - Shared domain concepts

The command maps between CLI arguments and Contract Models.

---

## 10. Testing

```cpp
// Test authorize command
TEST(AuthorizeCommandTest, ValidRequest_ReturnsSuccess) {
    // Arrange
    auto fraud_detection = std::make_unique<MockFraudDetectionService>();
    EXPECT_CALL(*fraud_detection, IsFraudulent(_, _, _))
        .WillOnce(::testing::Return(false));

    auto repository = std::make_unique<MockPaymentRepository>();
    EXPECT_CALL(*repository, Save(_, _, _, _))
        .WillOnce(::testing::Return(PaymentRecord{
            "test-id",
            Money(100, "USD"),
            "MERCH-001",
            PaymentStatus::Authorized,
            std::chrono::system_clock::now()
        }));

    auto maximum_amount = Money(10000, "USD");
    auto auth_service = std::make_unique<PaymentAuthorizationService>(
        fraud_detection.get(), maximum_amount);
    auto use_case = std::make_unique<AuthorizePaymentUseCase>(
        auth_service.get(), repository.get());

    cli::commands::AuthorizeCommand command(use_case.get());

    // Create parser from simulated arguments
    const char* args[] = {"payment-cli", "authorize", 
                          "--amount", "100", 
                          "--currency", "USD", 
                          "--merchant", "MERCH-001"};
    cli::CliParser parser(8, const_cast<char**>(args));

    // Act
    int exit_code = command.Execute(parser);

    // Assert
    ASSERT_EQ(exit_code, 0);
}
```

---

## 11. Summary

This example demonstrates:

1. **CLI argument parsing** - Simple but effective parser
2. **Command pattern** - Each command is a separate class
3. **Use Case invocation** - Direct, synchronous calls
4. **Thread lifecycle** - Short-lived: parse ? execute ? output ? exit
5. **Separation**: CLI args ? Contract Models ? Value Objects
6. **No business logic** - CLI only parses, invokes, and formats
7. **Standard Unix conventions** - Exit codes, help text, argument format

The pattern is simple: parse arguments ? map to contract ? invoke use case ? format output ? exit.

---

**See Also:**
- [C++ Implementation (AP-002)](AP-002-Implementation-Cpp.md) - Base implementation this builds on
- [C++ HTTP Controller Example](AP-003-HTTP-Controller-Cpp.md) - HTTP equivalent
- AP-003 - Incoming Implementations (specification)
