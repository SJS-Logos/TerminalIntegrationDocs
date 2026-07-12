# Payment Service Worker - MassTransit Consumer

This is a **standalone Worker Service** that demonstrates how to initialize messaging technology (MassTransit + RabbitMQ) close to the adapter layer.

## Key Architectural Points

### Technology Initialization at the Right Layer

The worker shows the principle: **Technology is initialized as close to the adapter as possible**.

```csharp
// Domain services - NO technology
builder.Services.AddSingleton<PaymentAuthorizationService>();

// Use cases - NO technology
builder.Services.AddScoped<AuthorizePaymentUseCase>();

// Adapters - Technology initialized HERE
builder.Services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();

// MassTransit - Technology initialized HERE (close to consumer/adapter)
builder.Services.AddMassTransit(x =>
{
    x.AddConsumer<AuthorizePaymentConsumer>();
    x.UsingRabbitMq((context, cfg) =>
    {
        cfg.Host("localhost", h =>
        {
            h.Username("guest");
            h.Password("guest");
        });
        // ... endpoint configuration
    });
});
```

**Domain and Application layers know nothing about:**
- MassTransit
- RabbitMQ
- Message queues
- Retry policies
- Serialization

**Only the Worker (composition root) knows about:**
- Which messaging technology to use
- How to connect to the broker
- Retry strategies
- Concurrency limits

## Running the Worker

### Prerequisites

- RabbitMQ running on localhost:5672 (default)

  **Docker:**
  ```bash
  docker run -d --name rabbitmq -p 5672:5672 -p 15672:15672 rabbitmq:3-management
  ```

  Management UI: http://localhost:15672 (guest/guest)

### Run the Worker

```bash
cd Logos.PaymentService.Worker
dotnet run
```

You should see:
```
info: MassTransit[0]
      Configured endpoint payment-authorization-queue
info: Microsoft.Hosting.Lifetime[0]
      Application started
```

### Send a Test Message

You can publish a test message using the WebApi or a simple publisher:

```csharp
// Test publisher
var busControl = Bus.Factory.CreateUsingRabbitMq(cfg =>
{
    cfg.Host("localhost");
});

await busControl.StartAsync();

await busControl.Publish(new AuthorizePaymentCommand
{
    MessageId = Guid.NewGuid(),
    Amount = 100m,
    Currency = "USD",
    MerchantId = "MERCH-001",
    CorrelationId = Guid.NewGuid()
});

await busControl.StopAsync();
```

The worker will:
1. Receive the message
2. Map it to `AuthorizePaymentRequest`
3. Execute `AuthorizePaymentUseCase` (synchronously)
4. Publish `PaymentAuthorizedEvent`
5. Log the result

## Project Structure

```
Logos.PaymentService.Worker/
|-- Program.cs                    # Composition root - technology initialization
|-- appsettings.json              # Configuration
`-- Logos.PaymentService.Worker.csproj
```

**Dependencies:**
- `Logos.PaymentService.Application` - Use cases (no technology)
- `Logos.PaymentService.Adapters` - Repository/service implementations
- `Logos.PaymentService.Messaging` - Consumer and message definitions
- `MassTransit` + `MassTransit.RabbitMQ` - Messaging technology

## Comparison to WebApi

| Aspect | WebApi | Worker |
|--------|--------|--------|
| **Technology** | ASP.NET Core + Kestrel | MassTransit + RabbitMQ |
| **Trigger** | HTTP request | Message from broker |
| **Initialization** | `Program.cs` configures Kestrel | `Program.cs` configures MassTransit |
| **Use Cases** | Same - `AuthorizePaymentUseCase` | Same - `AuthorizePaymentUseCase` |
| **Domain** | Same - `PaymentAuthorizationService` | Same - `PaymentAuthorizationService` |

Both are **incoming implementations** (AP-003). The domain and use cases are identical. Only the transport technology differs, and it's initialized in the composition root.

## Configuration

The worker uses `appsettings.json` for logging configuration:

```json
{
  "Logging": {
    "LogLevel": {
      "Default": "Information",
      "MassTransit": "Debug"
    }
  }
}
```

For production, you would add:
- Connection strings
- Retry policies
- Dead-letter configuration
- Monitoring/telemetry

## Benefits of This Approach

? **Technology isolation** - Domain/Application know nothing about MassTransit  
? **Testability** - Use cases can be tested without messaging infrastructure  
? **Flexibility** - Can swap RabbitMQ for Azure Service Bus by changing `Program.cs`  
? **Clear boundaries** - Technology decisions stay at the edges  
? **Same domain logic** - Whether triggered by HTTP or message, logic is identical  

This demonstrates the principle from AP-007: **Adapters translate between domain scope and technology scope**, and technology is initialized as close to the adapter as possible.
