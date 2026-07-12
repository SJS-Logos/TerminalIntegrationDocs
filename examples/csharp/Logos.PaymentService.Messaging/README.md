# Logos.PaymentService.Messaging

This is the MassTransit-based messaging service for the Payment Service. It's implemented as a .NET Worker Service that runs as a background service, consuming messages from RabbitMQ.

## Project Structure

- **Consumers/**: MassTransit message consumers
  - `AuthorizePaymentConsumer`: Consumes `AuthorizePaymentCommand` messages and publishes `PaymentAuthorizedEvent`
- **Messages/**: Message contracts
  - `AuthorizePaymentCommand`: Command to authorize a payment
  - `PaymentAuthorizedEvent`: Event published after payment authorization
- **MassTransitConfiguration.cs**: Extension method for configuring MassTransit with RabbitMQ
- **Program.cs**: Entry point that sets up the worker service

## Technology

- **MassTransit v8.2.5** (Apache 2.0 license)
- **RabbitMQ** as the message broker
- **.NET 8 Worker Service**

## Configuration

Configure RabbitMQ connection in `appsettings.json`:

```json
{
  "RabbitMQ": {
    "Host": "localhost",
    "VirtualHost": "/",
    "Username": "guest",
    "Password": "guest"
  }
}
```

## Running the Service

```bash
dotnet run --project Logos.PaymentService.Messaging
```

## Usage in Other Projects

You can reuse the MassTransit configuration in other projects by calling:

```csharp
builder.Services.AddPaymentServiceMessaging(builder.Configuration);
```

This will register all consumers and configure the RabbitMQ connection.
