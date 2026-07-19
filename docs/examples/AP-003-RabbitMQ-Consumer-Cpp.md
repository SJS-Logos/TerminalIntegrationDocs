# RabbitMQ Host Example - C++

**Version:** 0.1  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C++](AP-002-Implementation-Cpp.md)

---

## 1. Introduction

This example demonstrates how to add a message broker consumer Host Unit that receives messages from RabbitMQ and invokes use cases from the [C++ implementation example](AP-002-Implementation-Cpp.md). It is the C++ equivalent of the [RabbitMQ Host Example - C#](AP-003-MassTransit-Consumer-CSharp.md) and shares the same Core as the CLI and HTTP hosts.

The full source for this example lives at [`examples/cpp/logos_payment_service_rabbitmq_host/`](../../examples/cpp/logos_payment_service_rabbitmq_host/README.md).

**Key Points:**
- Consumes `AuthorizePaymentCommand` messages from a RabbitMQ queue
- Maps each message to an Application Contract Model
- Invokes the `AuthorizePaymentUseCase` synchronously
- Publishes a `PaymentAuthorizedEvent` describing the outcome
- Thread lifecycle: Deliver -> Map -> Invoke Use Case -> Map Result -> Publish -> Ack

Like every AP-003 transport endpoint, the consumer contains **no business logic** - all business decisions are made in the Domain, reached through the Use Case.

---

## 2. Project Structure

```
logos_payment_service_core/                     (From AP-002 example)
??? domain/
??? shared_kernel/
??? capabilities/
??? application/
logos_payment_service_rabbitmq_host/            (Message broker incoming)
??? messages/
?   ??? authorize_payment_command.h             # Incoming message contract
?   ??? payment_authorized_event.h              # Outgoing message contract
??? mappings/
?   ??? message_mapping.h
?   ??? message_mapping.cpp                      # JSON <-> message <-> contract (translational only)
??? consumers/
?   ??? authorize_payment_consumer.h
?   ??? authorize_payment_consumer.cpp           # Transport endpoint
??? configuration/
?   ??? rabbitmq_configuration.h                 # Connection settings (env-driven)
?   ??? rabbitmq_host.h
?   ??? rabbitmq_host.cpp                         # Broker wiring (AMQP-CPP + libev)
??? main.cpp                                      # Composition root
```

---

## 3. Message Contracts (Transport Models)

**Incoming - `AuthorizePaymentCommand`** (JSON):

```json
{
  "messageId": "3f2504e0-4f89-41d3-9a0c-0305e82c3301",
  "amount": "100.00",
  "currency": "USD",
  "merchantId": "MERCH-001",
  "correlationId": "8a1d..."
}
```

> `amount` may be sent as a string (`"100.00"`) or a JSON number (`100.00`).

**Outgoing - `PaymentAuthorizedEvent`** (JSON):

```json
{
  "paymentId": "PAY-000001",
  "isAuthorized": true,
  "status": "Authorized",
  "amount": "100.00",
  "currency": "USD",
  "declineReason": null,
  "authorizedAt": "2024-12-07T10:30:45Z",
  "correlationId": "8a1d..."
}
```

---

## 4. The Consumer (Transport Endpoint)

The consumer is the C++ equivalent of the C# `IConsumer<AuthorizePaymentCommand>`. It receives a raw payload, maps it to an Application Contract, invokes the Use Case, and produces a serialized `PaymentAuthorizedEvent` to publish. It holds a non-owning pointer to the Use Case and contains no business logic.

```cpp
// logos_payment_service_rabbitmq_host/consumers/authorize_payment_consumer.h
#pragma once

#include <string>

#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"

namespace logos::payment::service::rabbitmq_host::consumers {

/// @brief Transport endpoint (consumer) for payment authorization messages.
/// @note This consumer contains NO business logic. All business decisions are
///       made in the Domain layer, invoked via the Use Case (AP-003).
class AuthorizePaymentConsumer {
public:
    explicit AuthorizePaymentConsumer(
        core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case);

    /// @brief Handle a single delivered message.
    /// @param message_payload Raw JSON payload delivered by the broker.
    /// @return Serialized PaymentAuthorizedEvent JSON to be published.
    std::string Consume(const std::string& message_payload);

private:
    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case_;  // Non-owning
};

} // namespace logos::payment::service::rabbitmq_host::consumers
```

```cpp
// logos_payment_service_rabbitmq_host/consumers/authorize_payment_consumer.cpp
std::string AuthorizePaymentConsumer::Consume(const std::string& message_payload) {
    // Deserialize transport message.
    auto command_opt = mappings::MessageMapping::ParseAuthorizeCommand(message_payload);
    if (!command_opt) {
        // Invalid message: rethrow so the caller can dead-letter/retry.
        throw std::runtime_error("Invalid AuthorizePaymentCommand payload");
    }
    const auto& command = *command_opt;

    // Map transport message -> Contract Model.
    auto request = mappings::MessageMapping::ToContract(command);

    // Execute use case (synchronous). All business decisions happen here.
    auto result = authorize_use_case_->Execute(request);

    // Map result -> outgoing event and serialize for publishing.
    auto event = mappings::MessageMapping::ToEvent(result, command.correlation_id);
    return mappings::MessageMapping::SerializeEvent(event);
}
```

The consumer deliberately rethrows on failure so the broker layer can nack/dead-letter the message, mirroring the C# consumer rethrowing so MassTransit can retry.

---

## 5. Composition Root

`main.cpp` wires the Core (via the `ServiceContainer`), constructs the consumer, loads broker configuration, and starts the host. The Core dependencies are identical to the CLI and HTTP hosts.

```cpp
// Create the transport endpoint (consumer).
rabbitmq_host::consumers::AuthorizePaymentConsumer consumer(&authorize_use_case);

// Load broker configuration (mechanical wiring) and start the host.
auto config = rabbitmq_host::configuration::RabbitMqConfiguration::FromEnvironment();
rabbitmq_host::configuration::RabbitMqHost host(config, &consumer);

if (!host.Run()) {
    std::cerr << "Failed to start RabbitMQ host\n";
    return 1;
}
```

---

## 6. Comparison with the C# MassTransit Host

| Concern | C# (MassTransit) | C++ (this host) |
|---------|------------------|-----------------|
| Message consumer | `IConsumer<AuthorizePaymentCommand>` | `AuthorizePaymentConsumer` |
| Broker wiring | `AddMassTransit(...).UsingRabbitMq(...)` | `RabbitMqHost` (AMQP-CPP) |
| Serialization | MassTransit (built-in) | `nlohmann/json` via `MessageMapping` |
| DI container | `IServiceCollection` | `ServiceContainer` |
| Use case | `AuthorizePaymentUseCase` | `AuthorizePaymentUseCase` (identical Core) |
| Publish result | `context.Publish(PaymentAuthorizedEvent)` | `channel.publish(...)` |

---

## 7. Prerequisites, Configuration, and Building

The RabbitMQ host target is **optional** and is only built when its dependencies are present (the default CLI build works without them):

- [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP) (`amqpcpp`)
- [libev](http://software.schmorp.de/pkg/libev.html) (`ev`)
- [nlohmann/json](https://github.com/nlohmann/json) (`nlohmann_json`)
- A running RabbitMQ broker

Connection settings default to a local broker and can be overridden via environment variables (mirrors the C# `appsettings.json` `RabbitMQ` section):

| Variable | Default | Description |
|----------|---------|-------------|
| `RABBITMQ_HOST` | `localhost` | Broker hostname |
| `RABBITMQ_PORT` | `5672` | Broker port |
| `RABBITMQ_VHOST` | `/` | Virtual host |
| `RABBITMQ_USERNAME` | `guest` | Username |
| `RABBITMQ_PASSWORD` | `guest` | Password |
| `RABBITMQ_COMMAND_QUEUE` | `authorize-payment-command` | Consumed queue |
| `RABBITMQ_EVENT_EXCHANGE` | `payment-authorized-event` | Published exchange |

```bash
cd examples/cpp
mkdir -p build && cd build
cmake ..
cmake --build . --config Release

# If the dependencies were found, the target is produced:
./payment_rabbitmq_host
```

If the dependencies are not found, CMake skips the target and the rest of the build (CLI host) proceeds normally.

---

## 8. Summary

This example demonstrates:

1. **Message broker consumer** - Receives commands from a RabbitMQ queue
2. **Transport endpoint** - Maps message -> Contract Model -> Value Objects
3. **Use Case invocation** - Direct, synchronous call
4. **Result publishing** - Maps result to an outgoing event and publishes it
5. **No business logic** - The consumer only maps, invokes, and serializes
6. **Retry semantics** - Rethrow on failure so the broker can nack/dead-letter
7. **Identical Core** - Same Domain and Application as the CLI and HTTP hosts

The pattern is simple: consume message -> map to contract -> invoke use case -> map result -> publish -> ack.

---

**See Also:**
- [C++ Implementation (AP-002)](AP-002-Implementation-Cpp.md) - Base implementation this builds on
- [RabbitMQ Host Example - C#](AP-003-MassTransit-Consumer-CSharp.md) - C# equivalent
- [HTTP Host Example - C++](AP-003-HTTP-Controller-Cpp.md) - HTTP equivalent
- [CLI Host Example - C++](AP-003-CLI-Cpp.md) - CLI equivalent
- [RabbitMQ Host source and README](../../examples/cpp/logos_payment_service_rabbitmq_host/README.md) - Full runnable example
- AP-003 - Incoming Implementations (specification)

**End of Document**
