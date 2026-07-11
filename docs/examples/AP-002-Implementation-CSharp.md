# AP-002 Implementation Guide: C#

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-002, AP-004  

---

# 1. Introduction

This document demonstrates a concrete implementation of the service structure defined in AP-002 using C# and .NET.

It shows how to structure a service using the Domain and Application units, how to define shared Value Objects (per AP-004 §7), and how Domain Services perform business logic while state is managed externally.

This example uses a simplified payment authorization service to illustrate the concepts.

**Key Architectural Decisions:**

- **Stateless Domain Services**: Business logic operates on Value Objects without entity classes
- **External State Management**: Identity and state persistence handled by the repository (database)
- **Shared Value Objects**: Value Objects defined once in Domain, referenced by Application contracts (AP-004 §7)
- **Synchronous Request-Response**: Thread starts with an event (request), executes business logic, persists state, and ends with a message (response). No async in domain/application layers; threads are short-lived.

---

# 2. Project Structure

A C# service following AP-002 consists of the following projects:

```
Company.PaymentService/
??? Company.PaymentService.Domain/
?   ??? ValueObjects/          (Shared with contracts per AP-004 §7)
?   ??? Services/
?   ??? Abstractions/          (Capabilities)
??? Company.PaymentService.Application/
    ??? UseCases/
    ??? Contracts/             (References Domain ValueObjects)
```

The Application project references the Domain project. The Domain project has no project references to other service projects.

---

# 3. Domain Layer

## 3.1 Value Objects

Value Objects are immutable domain concepts shared between Domain and Application (AP-004 §7).

```csharp
// Company.PaymentService.Domain/ValueObjects/Money.cs
namespace Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Shared Value Object representing monetary amounts.
/// Used by both Domain logic and Application contracts (AP-004 §7).
/// </summary>
public record Money(decimal Amount, string Currency)
{
    public static Money Zero(string currency) => new(0, currency);

    public Money Add(Money other)
    {
        if (Currency != other.Currency)
            throw new InvalidOperationException("Cannot add money with different currencies");

        return new Money(Amount + other.Amount, Currency);
    }

    public bool IsPositive() => Amount > 0;

    public bool IsGreaterThan(Money other)
    {
        if (Currency != other.Currency)
            throw new InvalidOperationException("Cannot compare money with different currencies");

        return Amount > other.Amount;
    }
}
```

```csharp
// Company.PaymentService.Domain/ValueObjects/PaymentStatus.cs
namespace Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Shared Value Object representing payment authorization status.
/// </summary>
public enum PaymentStatus
{
    Pending,
    Authorized,
    Declined
}
```

```csharp
// Company.PaymentService.Domain/ValueObjects/PaymentRecord.cs
namespace Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Value Object representing a payment record.
/// The identity (Id) is assigned by the repository/database.
/// </summary>
public record PaymentRecord(
    Guid Id,
    Money Amount,
    string MerchantId,
    PaymentStatus Status,
    DateTime CreatedAt,
    string? DeclineReason = null);
```

## 3.2 Domain Services

Domain services contain business logic and work with Value Objects. They are stateless.

```csharp
// Company.PaymentService.Domain/Services/PaymentAuthorizationService.cs
namespace Company.PaymentService.Domain.Services;

using Company.PaymentService.Domain.ValueObjects;
using Company.PaymentService.Domain.Abstractions;

/// <summary>
/// Stateless service that performs payment authorization business logic.
/// Works with Value Objects; state persistence is handled by repository.
/// </summary>
public class PaymentAuthorizationService(IFraudDetectionService fraudDetection, Money maximumAmount)
{
    public PaymentAuthorizationResult Authorize(
        Money amount,
        string merchantId)
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

Capabilities are domain-owned abstractions that describe what the Domain requires.

```csharp
// Company.PaymentService.Domain/Abstractions/IFraudDetectionService.cs
namespace Company.PaymentService.Domain.Abstractions;

/// <summary>
/// Capability: Fraud detection expressed in domain terms.
/// </summary>
public interface IFraudDetectionService
{
    bool IsFraudulent(
        decimal amount,
        string currency,
        string merchantId);
}
```

```csharp
// Company.PaymentService.Domain/Abstractions/IPaymentRepository.cs
namespace Company.PaymentService.Domain.Abstractions;

using Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Capability: Persistence expressed in domain terms.
/// Responsible for identity generation and state management.
/// </summary>
public interface IPaymentRepository
{
    PaymentRecord? GetById(Guid id);

    /// <summary>
    /// Saves a payment record. The repository assigns the Id.
    /// </summary>
    PaymentRecord Save(
        Money amount,
        string merchantId,
        PaymentStatus status,
        string? declineReason);
}
```

---

# 4. Application Layer

## 4.1 Contract Models

Contract models reference shared Value Objects from the Domain (AP-004 §7).

```csharp
// Company.PaymentService.Application/Contracts/AuthorizePaymentRequest.cs
namespace Company.PaymentService.Application.Contracts;

using Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Contract model that references shared Value Objects.
/// No duplication - Money is defined once in Domain.
/// </summary>
public record AuthorizePaymentRequest(
    Money Amount,
    string MerchantId);
```

```csharp
// Company.PaymentService.Application/Contracts/AuthorizePaymentResponse.cs
namespace Company.PaymentService.Application.Contracts;

using Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Contract model that references shared Value Objects.
/// </summary>
public record AuthorizePaymentResponse(
    Guid PaymentId,
    bool IsAuthorized,
    PaymentStatus Status,
    Money Amount,
    string? DeclineReason);
```

```csharp
// Company.PaymentService.Application/Contracts/GetPaymentResponse.cs
namespace Company.PaymentService.Application.Contracts;

using Company.PaymentService.Domain.ValueObjects;

/// <summary>
/// Contract model for retrieving payment records.
/// </summary>
public record GetPaymentResponse(
    Guid PaymentId,
    Money Amount,
    string MerchantId,
    PaymentStatus Status,
    DateTime CreatedAt,
    string? DeclineReason);
```

## 4.2 Use Cases

Use cases orchestrate domain operations and manage transaction boundaries.

```csharp
// Company.PaymentService.Application/UseCases/AuthorizePaymentUseCase.cs
namespace Company.PaymentService.Application.UseCases;

using Company.PaymentService.Application.Contracts;
using Company.PaymentService.Domain.Services;
using Company.PaymentService.Domain.Abstractions;

/// <summary>
/// Use case that orchestrates payment authorization.
/// Domain service performs business logic.
/// Repository manages state and identity.
/// </summary>
public class AuthorizePaymentUseCase(
    PaymentAuthorizationService authorizationService,
    IPaymentRepository paymentRepository)

    {
        public async Task<AuthorizePaymentResponse> ExecuteAsync(
            AuthorizePaymentRequest request,
            CancellationToken cancellationToken)
        {
            // Execute business logic through stateless domain service
            var result = await authorizationService.AuthorizeAsync(
                request.Amount,
                request.MerchantId,
                cancellationToken);

            // Persist state through repository (repository assigns Id)
            var savedPayment = await paymentRepository.SaveAsync(
            request.Amount,
            request.MerchantId,
            result.Status,
            result.DeclineReason,
            cancellationToken);

        // Return contract model
        return new AuthorizePaymentResponse(
            savedPayment.Id,
            result.IsAuthorized,
            savedPayment.Status,
            savedPayment.Amount,
            savedPayment.DeclineReason);
    }
}
```

```csharp
// Company.PaymentService.Application/UseCases/GetPaymentUseCase.cs
namespace Company.PaymentService.Application.UseCases;

using Company.PaymentService.Application.Contracts;
using Company.PaymentService.Domain.Abstractions;

/// <summary>
/// Use case for retrieving payment records.
/// </summary>
public class GetPaymentUseCase(IPaymentRepository paymentRepository)
{
    public GetPaymentResponse? Execute(Guid paymentId)
    {
        var payment = paymentRepository.GetById(paymentId);

        if (payment == null)
            return null;

        return new GetPaymentResponse(
            payment.Id,
            payment.Amount,
            payment.MerchantId,
            payment.Status,
            payment.CreatedAt,
            payment.DeclineReason);
    }
}
```

---

# 5. Dependency Injection

Configuration happens at the composition root; adapters are registered by interface.

```csharp
// Composition root (typically in Program.cs)
var builder = WebApplication.CreateBuilder(args);

// Domain configuration - injected as parameters
var maximumPaymentAmount = new Money(
    builder.Configuration.GetValue<decimal>("Payment:MaximumAmount"),
    builder.Configuration.GetValue<string>("Payment:Currency") ?? "USD");

builder.Services.AddSingleton(maximumPaymentAmount);
builder.Services.AddScoped<PaymentAuthorizationService>();
builder.Services.AddScoped<AuthorizePaymentUseCase>();
builder.Services.AddScoped<GetPaymentUseCase>();

// Register capability implementations (details covered in later APs)
builder.Services.AddScoped<IPaymentRepository, EntityFrameworkPaymentRepository>();
builder.Services.AddScoped<IFraudDetectionService, ThirdPartyFraudDetectionService>();
```

---

# 6. Adapter Registration

Adapters implement domain capabilities. Their concrete implementations are outside the scope of AP-002 and will be covered in later APs on infrastructure.

The key point for AP-002 is:
- Domain defines **what** it needs (capabilities/abstractions)
- Adapters provide **how** it's implemented (covered in later APs)
- Composition root connects them

---

# 7. Key Points

1. **The Domain has no external dependencies**: `Company.PaymentService.Domain` references no other service projects.

2. **The Application references only the Domain**: `Company.PaymentService.Application` references only `Company.PaymentService.Domain`.

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
