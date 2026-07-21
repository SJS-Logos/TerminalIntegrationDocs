# C++ vs C# Implementation Comparison

This document compares the C++ and C# implementations of the Payment Service to highlight language-specific patterns and architectural similarities.

## Project Structure Comparison

### C# (examples/csharp/)
```
MyPaymentService.Domain/
MyPaymentService.Application/
MyPaymentService.Adapters/
MyPaymentService.WebApi/
MyPaymentService.Messaging/
MyPaymentService.Worker/
```

### C++ (examples/cpp/)
```
mypaymentservice_domain/
mypaymentservice_application/
mypaymentservice_adapters/
mypaymentservice_cli/
```

## Architecture Similarities

Both implementations follow the same architectural patterns:

1. **Domain Layer**: Pure business logic with no external dependencies
2. **Application Layer**: Use case orchestration depending only on Domain
3. **Adapters Layer**: Infrastructure implementations of domain abstractions
4. **Entry Points**: Different incoming adapters (WebAPI, CLI, MassTransit Worker)

## Language-Specific Differences

### Value Objects

**C#** (Reference Type with Records)
```csharp
public record Money(decimal Amount, string Currency)
{
    public static Money Zero(string currency) => new(0m, currency);
    public Money Add(Money other) => new(Amount + other.Amount, Currency);
    public bool IsPositive() => Amount > 0;
}
```

**C++** (Value Type with Integer Cents)
```cpp
class Money {
public:
    Money(double amount, const std::string& currency);  // Converts to cents
    static Money FromCents(int64_t cents, const std::string& currency);
    Money Add(const Money& other) const;
    bool IsPositive() const;

    double GetAmount() const { return cents_ / 100.0; }  // For display
    int64_t GetCents() const { return cents_; }          // For calculations
private:
    int64_t cents_;      // Stored as integer cents
    std::string currency_;
};
```

**Key Differences:**
- C# uses `decimal` for money (128-bit precise decimal type)
- C++ uses `int64_t` cents to avoid floating-point precision issues
- C# records provide automatic equality/immutability, C++ requires manual implementation
- C# is reference type by default, C++ is value type by default
- Both avoid floating-point (double/float) for money to prevent precision errors

### Dependency Injection

**C#** (Built-in Framework)
```csharp
builder.Services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
builder.Services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();
builder.Services.AddScoped<AuthorizePaymentUseCase>();
```

**C++** (Custom Container)
```cpp
container.Register<domain::abstractions::IPaymentRepository>(
    std::make_unique<adapters::InMemoryPaymentRepository>());
container.Register<domain::abstractions::IFraudDetectionService>(
    std::make_unique<adapters::SimpleFraudDetectionService>());
```

**Key Differences:**
- C# has built-in DI with lifetime management (Singleton, Scoped, Transient)
- C++ requires custom implementation or third-party library
- C# uses interfaces extensively, C++ uses abstract base classes

### Memory Management

**C#** (Garbage Collection)
```csharp
// No explicit memory management needed
var repository = new InMemoryPaymentRepository();
// GC handles cleanup automatically
```

**C++** (Explicit Ownership)
```cpp
// Explicit ownership with smart pointers
auto repository = std::make_unique<InMemoryPaymentRepository>();
// unique_ptr automatically deletes when out of scope

// Non-owning dependencies use raw pointers
PaymentAuthorizationService(IFraudDetectionService* fraud_detection)
```

**Key Differences:**
- C# relies on garbage collection
- C++ uses RAII and smart pointers (`unique_ptr`, `shared_ptr`)
- C++ follows ownership semantics (C++ Core Guidelines F.27)

### Error Handling

**C#** (Exceptions)
```csharp
if (currency != other.Currency)
    throw new InvalidOperationException("Cannot add different currencies");
```

**C++** (Exceptions + Optional)
```cpp
if (currency_ != other.currency_)
    throw std::runtime_error("Cannot add money with different currencies");

// Also uses std::optional for nullable returns
std::optional<PaymentRecord> GetById(const std::string& id);
```

**Key Differences:**
- Both use exceptions for error conditions
- C# uses nullable reference types (`Payment?`), C++ uses `std::optional<T>`
- C++ can also use error codes or `std::expected` (C++23)

### Repository Pattern

**C#**
```csharp
public interface IPaymentRepository
{
    void Save(Payment payment);
    Payment? GetById(string id);
}
```

**C++**
```cpp
class IPaymentRepository {
public:
    virtual ~IPaymentRepository() = default;
    virtual std::optional<PaymentRecord> GetById(const std::string& id) = 0;
    virtual PaymentRecord Save(...) = 0;
};
```

**Key Differences:**
- C# uses nullable reference types (`Payment?`)
- C++ uses `std::optional<T>` for optional returns
- C++ requires virtual destructor for polymorphism
- C++ uses pure virtual functions (`= 0`)

## Entry Point Comparison

### C# Web API
```csharp
[ApiController]
[Route("api/[controller]")]
public class PaymentsController : ControllerBase
{
    [HttpPost("authorize")]
    public IActionResult Authorize([FromBody] AuthorizePaymentRequest request)
    {
        var response = _useCase.Execute(request);
        return Ok(response);
    }
}
```

### C++ CLI
```cpp
int main(int argc, char* argv[]) {
    CliParser parser(argc, argv);

    if (parser.GetCommand() == "authorize") {
        AuthorizeCommand cmd(&authorize_use_case);
        return cmd.Execute(parser);
    }
}
```

**Key Differences:**
- C# example uses ASP.NET Core Web API (HTTP)
- C++ example uses command-line interface
- Both are thin adapters around the same use cases

### C# MassTransit Consumer
```csharp
public class AuthorizePaymentConsumer : IConsumer<AuthorizePaymentCommand>
{
    public async Task Consume(ConsumeContext<AuthorizePaymentCommand> context)
    {
        var response = _useCase.Execute(request);
        await context.Publish(new PaymentAuthorizedEvent(...));
    }
}
```

**C++ Equivalent** (not implemented yet)
```cpp
// Would use a messaging library like RabbitMQ C++ client
class AuthorizePaymentConsumer {
    void Consume(const AuthorizePaymentCommand& command) {
        auto response = use_case_->Execute(request);
        publisher_->Publish(PaymentAuthorizedEvent(...));
    }
};
```

## Performance Characteristics

| Aspect | C# | C++ |
|--------|----|----|
| **Startup Time** | Slower (JIT compilation) | Faster (ahead-of-time compiled) |
| **Memory Usage** | Higher (GC overhead) | Lower (precise control) |
| **Runtime Performance** | Good (optimized JIT) | Excellent (native code) |
| **Development Speed** | Faster (less boilerplate) | Slower (manual memory management) |
| **Type Safety** | Strong (compile-time) | Strong (compile-time) |
| **Platform Support** | Cross-platform (.NET) | Cross-platform (compile per platform) |

## When to Use Each

### Use C# When:
- Building web services/APIs
- Rapid development is priority
- Leveraging .NET ecosystem (Entity Framework, MassTransit, etc.)
- Team is more familiar with C#/.NET
- Memory overhead is acceptable
- Need automatic memory management

### Use C++ When:
- Maximum performance is critical
- Low memory footprint required
- Embedded systems or resource-constrained environments
- Interfacing with C libraries
- Need deterministic destruction (RAII)
- Team has C++ expertise
- Building system-level software

## Common Patterns in Both

Despite language differences, both implementations share:

1. **Same Business Logic**: Authorization rules are identical
2. **Stateless Services**: Domain services work on value objects
3. **External State**: Repository manages persistence
4. **Clear Boundaries**: Domain ? Application ? Adapters
5. **Testability**: Business logic isolated from infrastructure
6. **Shared Value Objects**: Contracts reference domain models

## Conclusion

Both implementations demonstrate that clean architecture patterns work across languages. The core business logic and architectural structure remain the same, with only language-specific implementation details differing.

Choose the language based on your requirements:
- **C#**: Better for web services, rapid development, .NET ecosystem
- **C++**: Better for performance-critical, low-level, or embedded systems

Both follow the same architectural principles defined in AP-002 and AP-003.
