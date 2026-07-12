# AP-002 Implementation Guide: C#

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-002, AP-004  

---

# 1. Introduction

This document demonstrates a concrete implementation of the service structure defined in AP-002 using C# and .NET.

It shows how to structure a service using the Domain and Application units, how to define shared Value Objects (per AP-004 §7), and how Domain Services perform business logic while state is managed externally.

This example uses a simplified payment authorization service to illustrate the concepts.

> **📦 Compilable Example Available**  
> A fully compilable version of this example is available at:  
> [`examples/csharp/`](../../examples/csharp/)  
>
> You can build and run it with:
> ```bash
> cd examples/csharp
> dotnet build
> cd Logos.PaymentService.WebApi
> dotnet run
> ```

**Key Architectural Decisions:**

- **Stateless Domain Services**: Business logic operates on Value Objects without entity classes
- **External State Management**: Identity and state persistence handled by the repository (database)
- **Shared Value Objects**: Value Objects defined once in Domain, referenced by Application contracts (AP-004 §7)
- **Synchronous Request-Response**: Thread starts with an event (request), executes business logic, persists state, and ends with a message (response). No async in domain/application layers; threads are short-lived.

---

# 2. Project Structure

The compilable example is organized into the following projects:

**Location:** [`examples/csharp/`](../../examples/csharp/)

```
Logos.PaymentService/
|-- Logos.PaymentService.Domain/
|   |-- Entities/
|   |-- ValueObjects/          (Shared with contracts per AP-004 §7)
|   |-- Services/
|   `-- Abstractions/          (Capabilities - AP-005)
`-- Logos.PaymentService.Application/
    |-- UseCases/
    `-- Contracts/             (References Domain ValueObjects)
```

See the [full README](../../examples/csharp/README.md) for the complete project structure including adapters and incoming implementations

The Application project references the Domain project. The Domain project has no project references to other service projects.

---

# 3. Domain Layer

## 3.1 Value Objects

Value Objects are immutable domain concepts shared between Domain and Application (AP-004 §7).

**File:** [`examples/csharp/Logos.PaymentService.Domain/ValueObjects/Money.cs`](../../examples/csharp/Logos.PaymentService.Domain/ValueObjects/Money.cs)

The `Money` value object:
- Represents monetary amounts with currency
- Immutable and validated at construction
- Provides domain operations (validation, comparison)
- Shared between Domain logic and Application contracts

**File:** [`examples/csharp/Logos.PaymentService.Domain/ValueObjects/PaymentStatus.cs`](../../examples/csharp/Logos.PaymentService.Domain/ValueObjects/PaymentStatus.cs)

The `PaymentStatus` enum represents the authorization state.

**File:** [`examples/csharp/Logos.PaymentService.Domain/Entities/Payment.cs`](../../examples/csharp/Logos.PaymentService.Domain/Entities/Payment.cs)

The `Payment` entity represents a payment with its state transitions.

## 3.2 Domain Services

Domain services contain business logic and work with entities and value objects.

**File:** [`examples/csharp/Logos.PaymentService.Domain/Services/PaymentAuthorizationService.cs`](../../examples/csharp/Logos.PaymentService.Domain/Services/PaymentAuthorizationService.cs)

The `PaymentAuthorizationService`:
- Stateless service with business logic
- Validates payment amount against maximum
- Checks for fraud via `IFraudDetectionService`
- Modifies payment entity state (authorize or decline)
    {
        if (!amount.IsPositive())
        {
            return new PaymentAuthorizationResult(
                IsAuthorized: false,
                Status: PaymentStatus.Declined,
                DeclineReason: "Amount must be positive");
        }

        // Business rule: Check for fraud
        var isFraudulent = fraudDetection.IsFraudulent(
            amount.Amount,
            amount.Currency,
            merchantId);

        if (isFraudulent)
        {
            return new PaymentAuthorizationResult(
                IsAuthorized: false,
                Status: PaymentStatus.Declined,
                DeclineReason: "Suspected fraud");
        }

        // Business rule: Amount limits
        if (amount.Currency == maximumAmount.Currency && 
            amount.IsGreaterThan(maximumAmount))
        {
            return new PaymentAuthorizationResult(
                IsAuthorized: false,
                Status: PaymentStatus.Declined,
                DeclineReason: "Amount exceeds maximum limit");
        }

        return new PaymentAuthorizationResult(
            IsAuthorized: true,
            Status: PaymentStatus.Authorized,
            DeclineReason: null);
    }
}

/// <summary>
/// Value Object representing the result of authorization business logic.
/// </summary>
public record PaymentAuthorizationResult(
    bool IsAuthorized,
    PaymentStatus Status,
    string? DeclineReason);
```

## 3.3 Capabilities (Domain Abstractions)

Capabilities are domain-owned abstractions that describe what the Domain requires (AP-005).

**File:** [`examples/csharp/Logos.PaymentService.Domain/Abstractions/IFraudDetectionService.cs`](../../examples/csharp/Logos.PaymentService.Domain/Abstractions/IFraudDetectionService.cs)

The `IFraudDetectionService` capability defines fraud detection in domain terms.

**File:** [`examples/csharp/Logos.PaymentService.Domain/Abstractions/IPaymentRepository.cs`](../../examples/csharp/Logos.PaymentService.Domain/Abstractions/IPaymentRepository.cs)

The `IPaymentRepository` capability defines persistence in domain terms.

---

# 4. Application Layer

## 4.1 Contract Models

Contract models reference shared Value Objects from the Domain (AP-004 §7).

**File:** [`examples/csharp/Logos.PaymentService.Application/Contracts/AuthorizePaymentContract.cs`](../../examples/csharp/Logos.PaymentService.Application/Contracts/AuthorizePaymentContract.cs)

Defines `AuthorizePaymentRequest` and `AuthorizePaymentResponse` records that reference the shared `Money` and `PaymentStatus` value objects from the domain.

**File:** [`examples/csharp/Logos.PaymentService.Application/Contracts/GetPaymentContract.cs`](../../examples/csharp/Logos.PaymentService.Application/Contracts/GetPaymentContract.cs)

Defines `GetPaymentRequest` and `GetPaymentResponse` records.

## 4.2 Use Cases

Use cases orchestrate domain operations and manage transaction boundaries.

**File:** [`examples/csharp/Logos.PaymentService.Application/UseCases/AuthorizePaymentUseCase.cs`](../../examples/csharp/Logos.PaymentService.Application/UseCases/AuthorizePaymentUseCase.cs)

The `AuthorizePaymentUseCase`:
- Receives `AuthorizePaymentRequest` contract
- Invokes `PaymentAuthorizationService` for business logic
- Persists result via `IPaymentRepository`
- Returns `AuthorizePaymentResponse` contract
- **Synchronous execution** on the incoming thread

**File:** [`examples/csharp/Logos.PaymentService.Application/UseCases/GetPaymentUseCase.cs`](../../examples/csharp/Logos.PaymentService.Application/UseCases/GetPaymentUseCase.cs)

The `GetPaymentUseCase` retrieves payment details from the repository.

---

# 5. Dependency Injection

Configuration happens at the composition root; adapters are registered by interface.

**File:** [`examples/csharp/Logos.PaymentService.WebApi/Program.cs`](../../examples/csharp/Logos.PaymentService.WebApi/Program.cs)

The composition root:
- Registers domain services with their dependencies
- Registers use cases
- Registers capability implementations (adapters)
- Configures dependency injection container

---

# 6. Adapter Registration

Adapters implement domain capabilities. Their concrete implementations are outside the scope of AP-002 and will be covered in later APs on infrastructure.

The key point for AP-002 is:
- Domain defines **what** it needs (capabilities/abstractions)
- Adapters provide **how** it's implemented (covered in later APs)
- Composition root connects them

---

# 7. Key Points

1. **The Domain has no external dependencies**: `Logos.PaymentService.Domain` references no other service projects.

2. **The Application references only the Domain**: `Logos.PaymentService.Application` references only `Logos.PaymentService.Domain`.

3. **Value Objects are shared** (AP-004 §7): `Money`, `PaymentStatus`, and `PaymentRecord` are defined once in the Domain and referenced by Application contracts. No duplication.

4. **Domain Services are stateless**: `PaymentAuthorizationService` performs business logic on Value Objects without managing state.

5. **State is external**: The repository (database) manages identity generation, persistence, and retrieval. The domain doesn't need entity classes for simple operations.

6. **Business logic is pure**: Authorization logic can be tested without any database or infrastructure.

7. **Capabilities define contracts**: `IPaymentRepository` and `IFraudDetectionService` express what the domain needs in domain terms.

8. **Configuration in composition root**: Domain services accept configuration as constructor parameters rather than hardcoding values, allowing flexibility and testability.

---

# 8. Testing

Business logic can be tested in complete isolation.

```csharp
// Domain Service test - pure business logic
[Fact]
public void Authorize_Declines_When_Amount_Exceeds_Maximum()
{
    // Arrange
    var fraudDetection = Substitute.For<IFraudDetectionService>();
    fraudDetection.IsFraudulent(Arg.Any<decimal>(), Arg.Any<string>(), Arg.Any<string>())
        .Returns(false);

    var maximumAmount = new Money(10000, "USD");
    var service = new PaymentAuthorizationService(fraudDetection, maximumAmount);
    var amount = new Money(15000, "USD"); // Exceeds limit

    // Act
    var result = service.Authorize(amount, "MERCH-001");

    // Assert
    Assert.False(result.IsAuthorized);
    Assert.Equal(PaymentStatus.Declined, result.Status);
    Assert.Contains("exceeds maximum", result.DeclineReason);
}

[Fact]
public void Authorize_Declines_When_Fraud_Detected()
{
    // Arrange
    var fraudDetection = Substitute.For<IFraudDetectionService>();
    fraudDetection.IsFraudulent(Arg.Any<decimal>(), Arg.Any<string>(), Arg.Any<string>())
        .Returns(true);

    var maximumAmount = new Money(10000, "USD");
    var service = new PaymentAuthorizationService(fraudDetection, maximumAmount);
    var amount = new Money(100, "USD");

    // Act
    var result = service.Authorize(amount, "MERCH-001");

    // Assert
    Assert.False(result.IsAuthorized);
    Assert.Equal(PaymentStatus.Declined, result.Status);
    Assert.Equal("Suspected fraud", result.DeclineReason);
}

// Value Object test
[Fact]
public void Money_Add_Returns_Correct_Total()
{
    var money1 = new Money(100.00m, "USD");
    var money2 = new Money(50.00m, "USD");

    var total = money1.Add(money2);

    Assert.Equal(150.00m, total.Amount);
    Assert.Equal("USD", total.Currency);
}

// Use Case test with mocked repository
[Fact]
public void AuthorizePayment_Saves_And_Returns_Result()
{
    // Arrange
    var fraudDetection = Substitute.For<IFraudDetectionService>();
    fraudDetection.IsFraudulent(Arg.Any<decimal>(), Arg.Any<string>(), Arg.Any<string>())
        .Returns(false);

    var repository = Substitute.For<IPaymentRepository>();
    var savedId = Guid.NewGuid();
    repository.Save(Arg.Any<Money>(), Arg.Any<string>(), Arg.Any<PaymentStatus>(),
        Arg.Any<string?>())
        .Returns(new PaymentRecord(
            savedId, 
            new Money(100, "USD"), 
            "MERCH-001", 
            PaymentStatus.Authorized, 
            DateTime.UtcNow));

    var maximumAmount = new Money(10000, "USD");
    var authService = new PaymentAuthorizationService(fraudDetection, maximumAmount);
    var useCase = new AuthorizePaymentUseCase(authService, repository);

    var request = new AuthorizePaymentRequest(new Money(100, "USD"), "MERCH-001");

    // Act
    var response = useCase.Execute(request);

    // Assert
    Assert.True(response.IsAuthorized);
    Assert.Equal(PaymentStatus.Authorized, response.Status);
    Assert.Equal(savedId, response.PaymentId);
}
```

---

# 9. When to Add Entities

This stateless approach works well for simple operations. Consider introducing entity classes when:

1. **Complex state transitions**: Multiple state changes with invariants that must be enforced
2. **Rich behavior**: Business logic that naturally belongs to the concept (not just validation)
3. **Aggregate boundaries**: Multiple related objects that must remain consistent
4. **Domain events**: Capturing what happened for integration or audit

For many services, the stateless approach with Value Objects and external persistence is sufficient and simpler.

---

# 10. References

- AP-001: Architectural Principles
- AP-002: Service Structure  
- AP-004: Domain (§7 Value Objects)
