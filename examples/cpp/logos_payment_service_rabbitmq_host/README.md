# Payment Service - RabbitMQ Host (C++)

The C++ equivalent of the C# MassTransit consumer example
([`Logos.Payment.Service.MasstransitHost`](../../csharp/Logos.Payment.Service.MasstransitHost/)).

This Host Unit is an **AP-003 Incoming Implementation** using a message broker
as its transport. It:

1. Consumes `AuthorizePaymentCommand` messages from a RabbitMQ queue
2. Maps each message to an Application Contract Model
3. Invokes the `AuthorizePaymentUseCase` (same Core as the CLI and HTTP hosts)
4. Publishes a `PaymentAuthorizedEvent` describing the outcome

Like every AP-003 transport endpoint, the consumer contains **no business
logic** - all decisions are made in the Domain, reached through the Use Case.

## Structure

```
logos_payment_service_rabbitmq_host/
+-- messages/
|   +-- authorize_payment_command.h     # Incoming message contract
|   +-- payment_authorized_event.h      # Outgoing message contract
+-- mappings/
|   +-- message_mapping.h/.cpp          # JSON <-> message <-> contract (translational only)
+-- consumers/
|   +-- authorize_payment_consumer.h/.cpp   # Transport endpoint
+-- configuration/
|   +-- rabbitmq_configuration.h        # Connection settings (env-driven)
|   +-- rabbitmq_host.h/.cpp            # Broker wiring (AMQP-CPP + libev)
+-- main.cpp                            # Composition root
```

## Comparison with the C# MassTransit host

| Concern | C# (MassTransit) | C++ (this host) |
|---------|------------------|-----------------|
| Message consumer | `IConsumer<AuthorizePaymentCommand>` | `AuthorizePaymentConsumer` |
| Broker wiring | `AddMassTransit(...).UsingRabbitMq(...)` | `RabbitMqHost` (AMQP-CPP) |
| Serialization | MassTransit (built-in) | `nlohmann/json` via `MessageMapping` |
| DI container | `IServiceCollection` | `ServiceContainer` |
| Use case | `AuthorizePaymentUseCase` | `AuthorizePaymentUseCase` (identical Core) |
| Publish result | `context.Publish(PaymentAuthorizedEvent)` | `channel.publish(...)` |

## Message contracts

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

## Prerequisites

The RabbitMQ host target is **optional** and is only built when its
dependencies are present (the default CLI build works without them):

- [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP) (`amqpcpp`)
- [libev](http://software.schmorp.de/pkg/libev.html) (`ev`)
- [nlohmann/json](https://github.com/nlohmann/json) (`nlohmann_json`)
- A running RabbitMQ broker

On Debian/Ubuntu:

```bash
sudo apt-get install libev-dev nlohmann-json3-dev
# AMQP-CPP is typically built from source or installed via vcpkg/conan
```

## Configuration

Connection settings default to a local broker and can be overridden via
environment variables (mirrors the C# `appsettings.json` `RabbitMQ` section):

| Variable | Default | Description |
|----------|---------|-------------|
| `RABBITMQ_HOST` | `localhost` | Broker hostname |
| `RABBITMQ_PORT` | `5672` | Broker port |
| `RABBITMQ_VHOST` | `/` | Virtual host |
| `RABBITMQ_USERNAME` | `guest` | Username |
| `RABBITMQ_PASSWORD` | `guest` | Password |
| `RABBITMQ_COMMAND_QUEUE` | `authorize-payment-command` | Consumed queue |
| `RABBITMQ_EVENT_EXCHANGE` | `payment-authorized-event` | Published exchange |

## Building and running

```bash
cd examples/cpp
mkdir -p build && cd build
cmake ..
cmake --build . --config Release

# If the dependencies were found, the target is produced:
./payment_rabbitmq_host
```

If the dependencies are not found, CMake prints:

```
amqpcpp / nlohmann_json / libev not found - skipping payment_rabbitmq_host target.
```

and the rest of the build (CLI host) proceeds normally.
