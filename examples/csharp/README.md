# Logos Payment Service - C# Compilable Example

This is a **fully compilable** example implementing the architectural patterns described in the AP series (AP-002, AP-003, AP-007).

## Project Structure

```
Logos.Payment/
|-- Logos.Payment.Core/                    # Core Unit (AP-002)
|   |-- Domain/
|   |   |-- Entities/
|   |   |   `-- Payment.cs
|   |   `-- Services/
|   |       `-- PaymentAuthorizationService.cs
|   |-- SharedKernel/                      # Shared business datatypes (AP-002)
|   |   |-- Money.cs
|   |   `-- PaymentStatus.cs
|   |-- Capabilities/                      # Domain-owned abstractions
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
|-- Logos.Payment.Infrastructure.InMemory/ # Infrastructure Unit (capability impls)
|   |-- InMemoryPaymentRepository.cs
|   `-- SimpleFraudDetectionService.cs
|
|-- Logos.Payment.HttpHost/                # HTTP Host Unit (AP-003)
|   |-- Controllers/
|   |   `-- PaymentsController.cs
|   |-- Mappings/
|   |   `-- PaymentDtos.cs
|   |-- Configuration/
|   |   `-- ServiceConfiguration.cs
|   `-- Program.cs
|
|-- Logos.Payment.MasstransitHost/         # MassTransit Host Unit (AP-003)
|   |-- Consumers/
|   |   `-- AuthorizePaymentConsumer.cs
|   |-- Messages/
|   |   |-- AuthorizePaymentCommand.cs
|   |   `-- PaymentAuthorizedEvent.cs
|   |-- Configuration/
|   |   `-- ServiceConfiguration.cs
|   `-- Program.cs
|
`-- Logos.Payment.CliHost/                 # CLI Host Unit (AP-003)
    |-- Commands/
    |   |-- AuthorizeCommand.cs
    |   `-- GetPaymentCommand.cs
    |-- Configuration/
    |   `-- ServiceConfiguration.cs
    |-- CliParser.cs
    `-- Program.cs
```

## Layer Separation

The example demonstrates **clean separation of concerns**:

### 1. Core Unit (Domain + SharedKernel + Capabilities + Application)
- `Domain/` contains business logic, entities, and domain services
- `SharedKernel/` holds shared immutable business datatypes (`Money`, `PaymentStatus`)
- `Capabilities/` defines capability interfaces (Ports)
- `Application/` contains use cases and contract models
- **No Host dependencies**

### 2. Infrastructure Unit (Depends on: Core)
- Implements capability interfaces
- Translates between domain and external systems
- Technology-specific implementations (`InMemory`)

### 3. Host Units (Depend on: Core, Infrastructure)
- HTTP controllers (`HttpHost`)
- Message consumers (`MasstransitHost`)
- CLI commands (`CliHost`)
- Transport endpoints + `Mappings/` + `Configuration/`
- Thin translation layer between transport and use cases
- **Technology initialized in composition root**

## Building and Running

### Prerequisites
- .NET 8.0 SDK or later

### Build
```bash
cd examples/csharp
dotnet build
```

### Run HTTP API
```bash
cd Logos.Payment.HttpHost
dotnet run
```

The API will start at `http://localhost:5000` with Swagger UI at the root.

Open your browser to: **http://localhost:5000**

### Run CLI

```bash
cd Logos.Payment.CliHost
dotnet run -- authorize --amount 100.00 --currency USD --merchant MERCH-001
dotnet run -- get <payment-id>
dotnet run -- help
```

See [Logos.Payment.CliHost/README.md](Logos.Payment.CliHost/README.md) for detailed CLI documentation.

### Test the API

**Authorize a Payment:**
```bash
curl -X POST http://localhost:5000/api/payments/authorize \
  -H "Content-Type: application/json" \
  -d '{
    "amount": 100.00,
    "currency": "USD",
    "merchantId": "MERCH-001"
  }'
```

**Get Payment Details:**
```bash
curl http://localhost:5000/api/payments/{payment-id}
```

## Key Architectural Features

### 1. **Synchronous Use Cases**
Use cases execute synchronously on the incoming thread:
```csharp
public AuthorizePaymentResponse Execute(AuthorizePaymentRequest request)
{
    var payment = new Payment(...);
    var isAuthorized = authorizationService.Authorize(payment);
    paymentRepository.Save(payment);
    return new AuthorizePaymentResponse(...);
}
```

Thread lifecycle: **Request -> Execute -> Response -> End**

### 2. **Explicit Value Objects**
Domain concepts are explicit types, not primitives:
```csharp
var amount = new Money(100m, "USD");  // Not just decimal
```

### 3. **Capability Pattern (Ports & Adapters)**
Domain defines interfaces; adapters implement them:
```csharp
// Domain defines the port
public interface IPaymentRepository { ... }

// Adapter implements it
public class InMemoryPaymentRepository : IPaymentRepository { ... }
```

### 4. **Direct Dependency Injection**
Use cases receive concrete dependencies via constructor:
```csharp
public class AuthorizePaymentUseCase(
    PaymentAuthorizationService authorizationService,  // Concrete
    IPaymentRepository paymentRepository)              // Interface
{
    // Use case implementation
}
```

No mediator, no unnecessary abstractions.

### 5. **Primary Constructors**
Modern C# syntax reduces boilerplate:
```csharp
public class PaymentAuthorizationService(
    IFraudDetectionService fraudDetectionService,
    Money maximumAmount)
{
    // Fields automatically created from parameters
}
```

## Testing Strategy

The architecture makes testing straightforward:

### Unit Tests (Domain)
Test domain logic in isolation:
```csharp
var fraudService = new FakeFraudDetectionService();
var maxAmount = new Money(10000m, "USD");
var service = new PaymentAuthorizationService(fraudService, maxAmount);

var payment = new Payment("PAY-001", new Money(100m, "USD"), "MERCH-001");
var result = service.Authorize(payment);

Assert.True(result);
```

### Integration Tests (Use Cases)
Test use cases with real or in-memory adapters:
```csharp
var repository = new InMemoryPaymentRepository();
var fraudService = new SimpleFraudDetectionService();
var authService = new PaymentAuthorizationService(fraudService, maxAmount);
var useCase = new AuthorizePaymentUseCase(authService, repository);

var request = new AuthorizePaymentRequest(...);
var response = useCase.Execute(request);

Assert.NotNull(response.PaymentId);
```

### API Tests (Controllers)
Test HTTP layer with `WebApplicationFactory`:
```csharp
var client = _factory.CreateClient();
var response = await client.PostAsJsonAsync("/api/payments/authorize", dto);
Assert.Equal(HttpStatusCode.OK, response.StatusCode);
```

## Extending the Example

### Add a New Use Case
1. Define contract models in `Core/Application/Contracts/`
2. Implement use case in `Core/Application/UseCases/`
3. Add controller endpoint in `HttpHost/Controllers/`

### Add a New Adapter
1. Implement the capability interface in an `Infrastructure.*` Unit
2. Register in the Host `Configuration/ServiceConfiguration.cs`

### Add MassTransit Consumer
1. Define message in `MasstransitHost/Messages/`
2. Implement consumer in `MasstransitHost/Consumers/`
3. Configure in `MasstransitHost/Configuration/ServiceConfiguration.cs`

## Documentation References

- **AP-002**: Service Structure -> [docs/AP-002.md](../../docs/AP-002.md)
- **AP-003**: Incoming Implementations -> [docs/AP-003.md](../../docs/AP-003.md)
- **AP-005**: Domain Capabilities -> [docs/AP-005.md](../../docs/AP-005.md)
- **AP-007**: Adapter Implementations -> [docs/AP-007.md](../../docs/AP-007.md)

## Benefits of This Approach

- **Compilable and Verifiable** - Every example can be built and tested
- **Simple** - Clean layer separation makes each component easy to understand
- **Testable** - Domain and use cases have no framework dependencies
- **Maintainable** - Changes are localized to appropriate layers
- **Realistic** - Uses real frameworks (ASP.NET Core, MassTransit v8)
- **Extensible** - Easy to add new use cases, adapters, or transports

## License

This example code is provided for educational purposes as part of the Logos Payment Solution documentation.
