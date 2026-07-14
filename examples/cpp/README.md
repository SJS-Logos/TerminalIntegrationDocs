# Logos Payment Service - C++ Example

This is a complete C++ implementation of the payment service following the architectural patterns defined in AP-002 (Domain/Application structure) and AP-003 (CLI implementation).

## Project Structure

```
examples/cpp/
??? logos_payment_core/                    # Core Unit (AP-002)
?   ??? domain/                            # Domain layer (business logic)
?   ?   ??? services/                      # Stateless business logic
?   ?       ??? payment_authorization_service.h/cpp
?   ??? shared_kernel/                     # Shared business datatypes
?   ?   ??? money.h/cpp
?   ?   ??? payment_status.h
?   ?   ??? payment_record.h
?   ??? capabilities/                      # Domain-owned interfaces
?   ?   ??? fraud_detection_service.h
?   ?   ??? payment_repository.h
?   ??? application/                       # Application layer (orchestration)
?       ??? contracts/                     # Request/Response DTOs
?       ?   ??? authorize_payment_request.h
?       ?   ??? authorize_payment_response.h
?       ?   ??? get_payment_response.h
?       ??? use_cases/                     # Application workflows
?       ?   ??? authorize_payment_use_case.h/cpp
?       ?   ??? get_payment_use_case.h/cpp
?       ??? container/                     # Dependency injection
?           ??? service_container.h
??? logos_payment_infrastructure/          # Infrastructure Unit (capability impls)
?   ??? in_memory_payment_repository.h/cpp
?   ??? simple_fraud_detection_service.h/cpp
??? logos_payment_cli_host/                # CLI Host Unit (AP-003)
    ??? cli_parser.h/cpp
    ??? commands/
    ?   ??? authorize_command.h/cpp
    ?   ??? get_payment_command.h/cpp
    ??? main.cpp
```

## Key Architectural Features

- **Stateless Domain Services**: Business logic operates on Value Objects without entity classes
- **External State Management**: Identity and state persistence handled by repository
- **Shared Value Objects**: Value Objects defined once in Domain, referenced by Application contracts
- **Template-Based Service Container**: Type-safe dependency injection
- **Synchronous Request-Response**: Thread lifecycle: Parse ? Execute ? Output ? Exit
- **Explicit Ownership**: Uses C++ Core Guidelines (unique_ptr for ownership, raw pointers for dependencies)
- **Precise Money Handling**: Uses integer cents (int64_t) to avoid floating-point precision issues

## Prerequisites

- CMake 3.20 or higher
- C++17 compatible compiler:
  - GCC 7+ / Clang 5+ / MSVC 2017+
  - Visual Studio 2019 or higher (on Windows)

## Building the Project

### Windows (Visual Studio)

```powershell
# Navigate to the cpp directory
cd examples/cpp

# Create build directory
mkdir build
cd build

# Generate Visual Studio solution
cmake ..

# Build the project
cmake --build . --config Release

# Run the CLI
.\Release\payment_cli.exe help
```

### Linux/macOS

```bash
# Navigate to the cpp directory
cd examples/cpp

# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake ..

# Build the project
make

# Run the CLI
./payment_cli help
```

## Usage Examples

### Authorize a Payment

```bash
# Authorize a valid payment
payment_cli authorize --amount 100.00 --currency USD --merchant MERCH-001

# Expected output:
# === Payment Authorization Result ===
# Payment ID:     PAY-000001
# Authorized:     YES
# Status:         Authorized
# Amount:         100.00 USD
# ====================================
```

### Test Fraud Detection

```bash
# Try to authorize a payment over $5000 (fraud threshold)
payment_cli authorize --amount 6000.00 --currency USD --merchant MERCH-001

# Expected output:
# === Payment Authorization Result ===
# Payment ID:     PAY-000002
# Authorized:     NO
# Status:         Declined
# Amount:         6000.00 USD
# Decline Reason: Suspected fraud
# ====================================
```

### Test Maximum Amount Limit

```bash
# Try to authorize a payment over $10000 (maximum limit)
payment_cli authorize --amount 15000.00 --currency USD --merchant MERCH-001

# Expected output:
# === Payment Authorization Result ===
# Payment ID:     PAY-000003
# Authorized:     NO
# Status:         Declined
# Amount:         15000.00 USD
# Decline Reason: Amount exceeds maximum limit
# ====================================
```

### Retrieve Payment Details

```bash
# Get details of a previously authorized payment
payment_cli get PAY-000001

# Expected output:
# === Payment Details ===
# Payment ID:     PAY-000001
# Amount:         100.00 USD
# Merchant ID:    MERCH-001
# Status:         Authorized
# Created At:     2024-01-15 10:30:45
# =======================
```

### Get Help

```bash
# General help
payment_cli help

# Command-specific help
payment_cli authorize --help
payment_cli get --help
```

## Architecture Highlights

### Money Value Object - Precise Currency Handling

The `Money` value object uses **integer cents** (int64_t) with proper formatting for display:

```cpp
// Construct from string (supports both "10.50" and "10,50")
Money amount("1234.56", "USD");

// Preferred: construct from cents directly
Money precise = Money::FromCents(123456, "USD");

// Get cents for calculations (exact integer arithmetic)
int64_t cents = amount.GetCents();  // Returns 123456

// Format for display with thousands separators and currency
std::string display = amount.ToString();  // Returns "1,234.56 USD"

// International formatting
std::string eu_format = amount.ToString(',', '.', true);  // "1.234,56 USD"
std::string us_format = amount.ToString('.', ',', true);  // "1,234.56 USD"
```

**Why Integer Cents?**
- Floating-point (double) cannot precisely represent 0.1, leading to errors like `0.1 + 0.2 ? 0.3`
- Financial calculations require exact precision
- Integer arithmetic is exact and faster
- Follows industry best practices for monetary values

**Why No Double in Public API?**
- Exposing `double` promotes incorrect usage patterns
- `ToString()` provides proper formatting with thousands separators
- Supports international number formats (US: 1,234.56 vs EU: 1.234,56)
- Currency is always included to avoid confusion

**Example Display:**
```cpp
Money large = Money::FromCents(123456789, "USD");
std::cout << large.ToString() << "\n";  // Output: "1,234,567.89 USD"
```

### Domain Layer (No External Dependencies)

The domain layer contains pure business logic with zero dependencies on infrastructure:

- **Shared Kernel**: Shared immutable business datatypes (Money, PaymentStatus, PaymentRecord)
- **Domain Services**: Stateless business logic (PaymentAuthorizationService)
- **Capabilities**: Domain-owned interfaces describing capabilities

### Application Layer (Depends Only on Domain)

The application layer orchestrates domain services:

- **Use Cases**: Coordinate domain services and persistence
- **Contracts**: Request/Response models using shared kernel value objects
- **Service Container**: Simple dependency injection

### Infrastructure Unit (Implements Domain Capabilities)

The infrastructure unit provides concrete implementations:

- **InMemoryPaymentRepository**: Simple in-memory storage for demo
- **SimpleFraudDetectionService**: Basic fraud detection (flags amounts > $5000)

### CLI Layer (Entry Point)

The CLI layer wires everything together:

- **CLI Parser**: Parses command-line arguments
- **Commands**: Map CLI input to use case contracts
- **Main**: Composition root that configures the service container

## Testing

The architecture enables easy testing. Domain services can be tested in isolation using mock objects:

```cpp
// Example test (requires GoogleTest/GoogleMock)
TEST(PaymentAuthorizationServiceTest, Authorize_Declines_When_Amount_Exceeds_Maximum) {
    MockFraudDetectionService fraud_detection;
    EXPECT_CALL(fraud_detection, IsFraudulent(_, _, _))
        .WillOnce(Return(false));

    Money maximum_amount(10000.0, "USD");
    PaymentAuthorizationService service(&fraud_detection, maximum_amount);
    Money amount(15000.0, "USD");  // Exceeds limit

    auto result = service.Authorize(amount, "MERCH-001");

    ASSERT_FALSE(result.is_authorized);
    ASSERT_EQ(result.status, PaymentStatus::Declined);
}
```

## Extending the Example

### Add a New Adapter

To add a real database or external fraud detection service:

1. Create a new class implementing the domain abstraction
2. Register it in the service container in `main.cpp`
3. No changes needed to domain or application layers

### Add a New Use Case

1. Create a new use case in `logos_payment_core/application/use_cases/`
2. Define contracts in `logos_payment_core/application/contracts/`
3. Create a CLI command in `logos_payment_cli_host/commands/`
4. Wire it up in `main.cpp`

## References

- [AP-002 Implementation Guide: C++](../../docs/examples/AP-002-Implementation-Cpp.md)
- [CLI Application Example: C++](../../docs/examples/AP-003-CLI-Cpp.md)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)

## License

This example is part of the Logos Payment Service documentation.
