# AP-003 Implementation Guide: MassTransit Consumer - C#

# AP-003 Implementation Guide: MassTransit Consumer - C#

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C#](AP-002-Implementation-CSharp.md)

---

## 1. Introduction

This example demonstrates how to add a MassTransit consumer (message handler) that receives messages and invokes use cases from the [C# implementation example](AP-002-Implementation-CSharp.md).

> **?? Compilable Example Available**  
> A fully compilable version of this example is available at:  
> `Logos.Payment.MasstransitHost/`  
>
> The consumer implementation is ready to use. See the [README](../../examples/csharp/README.md) for integration instructions.

**Key Points:**
- Consumer receives message from broker (RabbitMQ, Azure Service Bus, etc.)
- Maps message payload to Contract Model (Request)
- Invokes Use Case synchronously
- Publishes result or acknowledgment
- Thread lifecycle: Message -> Execute -> Publish -> End

**MassTransit Version:** v8.x (Apache 2.0 license)

---

## 2. Project Structure

**Location:** `Logos.Payment.MasstransitHost/`

The Host Unit is a standalone application that shows how technology is initialized close to the incoming implementation:

```
Logos.Payment.MasstransitHost/
|-- Consumers/
|   `-- AuthorizePaymentConsumer.cs
|-- Mappings/
|   `-- AuthorizePaymentMessageMapping.cs
|-- Configuration/
|   `-- ServiceConfiguration.cs
|-- Program.cs
`-- appsettings.json
```

The Host depends on:
- `Logos.Payment.Core` - Use cases and contracts
- `MassTransit` + `MassTransit.RabbitMQ` - Messaging technology

**Key Point:** Technology (MassTransit, RabbitMQ) is initialized in `Program.cs`, close to where it's used, NOT in the domain or application layers.

---

## 3. Message Definitions

### 3.1 Command Message

**File:** `Logos.Payment.MasstransitHost/Mappings/AuthorizePaymentCommand.cs`

The command message:
- Represents the incoming message from the broker
- Maps to `AuthorizePaymentRequest` domain contract
- Includes `MessageId` for idempotency
- Includes `CorrelationId` for distributed tracing

### 3.2 Event Message

**File:** `Logos.Payment.MasstransitHost/Mappings/PaymentAuthorizedEvent.cs`

The event message:
- Published after payment authorization completes
- Contains authorization result (approved/declined)
- Includes correlation ID for tracing

---

## 4. Consumer Implementation

**File:** `Logos.Payment.MasstransitHost/Consumers/AuthorizePaymentConsumer.cs`

The consumer:
- Implements `IConsumer<AuthorizePaymentCommand>`
- Maps message -> domain contract using shared `Money` value object
- Invokes `AuthorizePaymentUseCase` **synchronously**
- Publishes `PaymentAuthorizedEvent` via MassTransit
- Logs with correlation IDs
- Throws exceptions to trigger retry policies

**Flow:**
```
1. Message arrives from broker
2. Map message to AuthorizePaymentRequest
3. Execute use case synchronously (returns result)
4. Publish PaymentAuthorizedEvent
5. Thread ends
```

---

## 5. Key Architectural Points

### 5.1 Translation Responsibility

- **Consumers are transport endpoints** - they receive messages and invoke use cases
- Message schemas are external contracts
- Domain contracts are internal
- Flow: `external message -> domain contract -> use case -> event`

### 5.2 Thread Lifecycle

- Thread is **started by message arrival** from broker
- Use case executes **synchronously** on that thread
- Event publishing is async but doesn't block the use case
- Thread **ends after** message processing completes

**Pattern:** Message -> Execute -> Publish -> End

### 5.3 Idempotency

- Use `MessageId` for idempotency checks
- Implement at the adapter/infrastructure level
- Domain use cases remain stateless

### 5.4 Error Handling

- Throw exceptions to trigger MassTransit retry policies
- Configure retry strategies in MassTransit setup
- Use dead-letter queues for permanently failed messages
- Log correlation IDs for distributed tracing

### 5.5 Technology Placement

- MassTransit configuration belongs in the Host Unit's `Configuration/` folder
- Consumers are **incoming transport endpoints** (AP-003)
- Message definitions are transport contracts, not domain contracts
- Use cases remain independent of messaging technology

---

## 6. Comparison to HTTP Controllers

| Aspect | HTTP Controller | MassTransit Consumer |
|--------|----------------|---------------------|
| **Trigger** | HTTP request | Message arrival from broker |
| **Response** | Synchronous HTTP response | Event publication |
| **Retry** | Client-controlled | Broker-controlled with retry policies |
| **Concurrency** | Per-request threads | Configurable prefetch/concurrency |
| **Idempotency** | Manual (check headers/DB) | Built-in (MessageId) |
| **Thread Lifecycle** | Request -> Execute -> Response -> End | Message -> Execute -> Publish -> End |

Both follow the same pattern:
1. **Receive** external input
2. **Map** to domain contract
3. **Execute** use case
4. **Map** result back
5. **Respond** or publish

---

## 7. Summary

This example demonstrates:

1. **Thin adapter layer** - Consumer only maps and invokes, no logic
2. **Synchronous use case execution** - Use case runs on the consumer thread
3. **Direct invocation** - No mediator, no command bus
4. **Message != Contract** - External message format is separate from internal contracts
5. **Technology isolation** - MassTransit details stay in the adapter layer

The pattern is simple: **receive message -> map to contract -> invoke use case -> publish event -> end**.

---

**See Also:**
- [C# Implementation (AP-002)](AP-002-Implementation-CSharp.md) - Base implementation
- [HTTP Controller Example](AP-003-HTTP-Controller-CSharp.md) - Synchronous incoming implementation
- [Compilable Example README](../../examples/csharp/README.md) - Full build instructions
- AP-003 - Incoming Implementations (specification)
- AP-007 - Adapter Implementations (specification)

---

## 8. References

[1] "MassTransit Documentation," https://masstransit.io/

[2] "AP-002: Service Structure," this series.

[3] "AP-003: Incoming Implementations," this series.

[4] "AP-007: Adapter Implementations," this series.
