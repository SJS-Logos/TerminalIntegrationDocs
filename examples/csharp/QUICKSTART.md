# Quick Start - Compilable C# Example

This guide gets you up and running with the compilable payment service example in **under 5 minutes**.

## Prerequisites

- [.NET 8.0 SDK](https://dotnet.microsoft.com/download) or later
- Any code editor (VS Code, Visual Studio, Rider, etc.)
- Git (to clone the repository)

## Quick Start

### 1. Clone the Repository
```bash
git clone https://github.com/Logos-Payment-Solution/TerminalIntegrationDocs
cd TerminalIntegrationDocs/examples/csharp
```

### 2. Build the Solution
```bash
dotnet build
```

Expected output:
```
Build succeeded
```

### 3. Run the API
```bash
cd Logos.Payment.HttpHost
dotnet run
```

The API will start. Look for output like:
```
Now listening on: http://localhost:5000
```

### 4. Test It!

#### Option A: Use Swagger UI
Open your browser to: **http://localhost:5000**

The Swagger UI will load automatically at the root. Try the endpoints interactively!

#### Option B: Use curl

**Authorize a payment:**
```bash
curl -X POST http://localhost:5000/api/payments/authorize \
  -H "Content-Type: application/json" \
  -d '{
    "amount": 100.00,
    "currency": "USD",
    "merchantId": "MERCH-001"
  }'
```

Response:
```json
{
  "paymentId": "550e8400-e29b-41d4-a716-446655440000",
  "isAuthorized": true,
  "status": "Authorized",
  "amount": 100.00,
  "currency": "USD",
  "merchantId": "MERCH-001",
  "declineReason": null
}
```

**Get payment details:**
```bash
curl http://localhost:5000/api/payments/{payment-id}
```

## What You're Running

### Architecture Overview
```
HTTP Request
POST /api/payments/authorize
    |
    v
PaymentsController (Incoming Implementation)
- Maps DTO -> Contract
- Invokes Use Case
- Maps Result -> DTO
    |
    v
AuthorizePaymentUseCase (Application)
- Creates Payment
- Authorizes via Domain Service
- Saves via Repository
    |
    v
PaymentAuthorizationService (Domain)
- Business logic
- Fraud check
- Amount validation
```

### Layer Responsibilities

**Domain** (Pure Business Logic)
- `Payment` entity
- `Money` value object
- `PaymentAuthorizationService` - business rules
- Interfaces: `IPaymentRepository`, `IFraudDetectionService`

**Application** (Use Case Orchestration)
- `AuthorizePaymentUseCase` - orchestrates domain operations
- `GetPaymentUseCase` - retrieves payment details
- Contract models (Request/Response records)

**Infrastructure** (Technology Translation)
- `InMemoryPaymentRepository` - stores payments in memory
- `SimpleFraudDetectionService` - simple fraud check (>$5000)

**HttpHost** (HTTP Incoming)
- `PaymentsController` - exposes HTTP endpoints
- DTO mapping between HTTP and domain contracts

## Project Structure

```
Logos.Payment.sln
|
|-- Logos.Payment.Core/                    # No Host dependencies
|   |-- Domain/
|   |   |-- Entities/
|   |   |   `-- Payment.cs
|   |   `-- Services/
|   |       `-- PaymentAuthorizationService.cs
|   |-- SharedKernel/
|   |   |-- Money.cs
|   |   `-- PaymentStatus.cs
|   |-- Capabilities/
|   |   |-- IPaymentRepository.cs
|   |   `-- IFraudDetectionService.cs
|   `-- Application/
|       |-- Contracts/
|       |   |-- AuthorizePaymentContract.cs
|       |   `-- GetPaymentContract.cs
|       `-- UseCases/
|           |-- AuthorizePaymentUseCase.cs
|           `-- GetPaymentUseCase.cs
|
|-- Logos.Payment.Infrastructure.InMemory/ # Depends on: Core
|   |-- InMemoryPaymentRepository.cs
|   `-- SimpleFraudDetectionService.cs
|
|-- Logos.Payment.HttpHost/                # Depends on: Core, Infrastructure
|   |-- Controllers/
|   |   `-- PaymentsController.cs
|   |-- Mappings/
|   |   `-- PaymentDtos.cs
|   |-- Configuration/
|   |   `-- ServiceConfiguration.cs
|   `-- Program.cs
|
`-- Logos.Payment.MasstransitHost/         # Depends on: Core, Infrastructure
    |-- Messages/
    |   |-- AuthorizePaymentCommand.cs
    |   `-- PaymentAuthorizedEvent.cs
    |-- Consumers/
    |   `-- AuthorizePaymentConsumer.cs
    |-- Configuration/
    |   `-- ServiceConfiguration.cs
    `-- Program.cs
```

## Key Patterns Demonstrated

### 1. Synchronous Use Cases
```csharp
public AuthorizePaymentResponse Execute(AuthorizePaymentRequest request)
{
    // Thread: Request -> Execute -> Response -> End
    var payment = new Payment(Guid.NewGuid().ToString(), request.Amount, request.MerchantId);
    var isAuthorized = authorizationService.Authorize(payment);
    paymentRepository.Save(payment);
    return new AuthorizePaymentResponse(...);
}
```

### 2. Explicit Value Objects
```csharp
// Money is a domain concept, not primitives
var amount = new Money(100m, "USD");
```

### 3. Capability Pattern (Ports & Adapters)
```csharp
// Domain defines the port
public interface IPaymentRepository { ... }

// Adapter implements it
public class InMemoryPaymentRepository : IPaymentRepository { ... }
```

### 4. Direct Injection
```csharp
// No mediator - use cases injected directly
public PaymentsController(
    AuthorizePaymentUseCase authorizeUseCase,
    GetPaymentUseCase getUseCase)
```

### 5. Primary Constructors
```csharp
// Modern C# - parameters become fields automatically
public class AuthorizePaymentUseCase(
    PaymentAuthorizationService authorizationService,
    IPaymentRepository paymentRepository)
{
    // authorizationService and paymentRepository are accessible
}
```

## Exploring the Code

### Start with Domain
Open `Domain/Services/PaymentAuthorizationService.cs` to see the core business logic:
```csharp
public bool Authorize(Payment payment)
{
    if (payment.Amount.GetAmount() > maximumAmount.GetAmount())
    {
        payment.Decline("Amount exceeds maximum");
        return false;
    }
    // ... fraud check ...
    payment.Authorize();
    return true;
}
```

### Then Application
Open `Application/UseCases/AuthorizePaymentUseCase.cs` to see orchestration:
```csharp
public AuthorizePaymentResponse Execute(AuthorizePaymentRequest request)
{
    var payment = new Payment(...);
    var isAuthorized = authorizationService.Authorize(payment);
    paymentRepository.Save(payment);
    return new AuthorizePaymentResponse(...);
}
```

### Finally HTTP Layer
Open `HttpHost/Controllers/PaymentsController.cs` to see HTTP translation:
```csharp
[HttpPost("authorize")]
public IActionResult AuthorizePayment([FromBody] AuthorizePaymentDto request)
{
    var useCaseRequest = new AuthorizePaymentRequest(...);
    var result = authorizePaymentUseCase.Execute(useCaseRequest);
    return Ok(new AuthorizePaymentDto { ... });
}
```

## Next Steps

### Modify the Code
Try changing the fraud detection threshold:
1. Open `Logos.Payment.Infrastructure.InMemory/SimpleFraudDetectionService.cs`
2. Change `amount.GetAmount() > 5000m` to `> 1000m`
3. Rebuild: `dotnet build`
4. Test with different amounts

### Add a New Use Case
Follow the pattern:
1. Define contracts in `Core/Application/Contracts/`
2. Implement use case in `Core/Application/UseCases/`
3. Add controller endpoint in `HttpHost/Controllers/`
4. Test via Swagger

### Run Tests (Coming Soon)
```bash
dotnet test
```

## Troubleshooting

### Build Fails
```bash
# Clean and rebuild
dotnet clean
dotnet build
```

### Port Already in Use
Edit `HttpHost/appsettings.json` or run:
```bash
dotnet run --urls "http://localhost:5002"
```

### Can't Connect
Make sure you're using HTTP (not HTTPS) and navigating to the root:
- Swagger UI: `http://localhost:5000`
- API endpoint: `http://localhost:5000/api/payments/...`

## Documentation

- [Full README](README.md) - Complete documentation
- [Compilable Examples Strategy](../../docs/COMPILABLE-EXAMPLES.md)
- [AP-002: Service Structure](../../docs/AP-002.md)
- [AP-003: Incoming Implementations](../../docs/AP-003.md)

## Questions?

This example demonstrates the architectural patterns from the AP series in working code. Each layer is clean, focused, and testable.

**Key takeaway:** Good architecture is **simple**. Clear separation, explicit dependencies, and direct invocation.
