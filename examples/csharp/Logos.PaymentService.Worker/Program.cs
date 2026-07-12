using MassTransit;
using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Services;
using Logos.PaymentService.Domain.ValueObjects;
using Logos.PaymentService.Adapters;
using Logos.PaymentService.Messaging.Consumers;

var builder = Host.CreateApplicationBuilder(args);

// Register Domain services (business logic - no technology)
var maxAmount = new Money(10000m, "USD");
builder.Services.AddSingleton(maxAmount);
builder.Services.AddSingleton<PaymentAuthorizationService>();

// Register Use Cases (orchestration - no technology)
builder.Services.AddScoped<AuthorizePaymentUseCase>();
builder.Services.AddScoped<GetPaymentUseCase>();

// Register Adapters (technology initialization happens HERE)
builder.Services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
builder.Services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

// Register MassTransit (technology initialized close to the adapter/consumer)
builder.Services.AddMassTransit(x =>
{
    // Register consumers
    x.AddConsumer<AuthorizePaymentConsumer>();

    // Configure RabbitMQ transport (technology details)
    x.UsingRabbitMq((context, cfg) =>
    {
        cfg.Host("localhost", h =>
        {
            h.Username("guest");
            h.Password("guest");
        });

        // Configure retry policy
        cfg.UseMessageRetry(r => r.Incremental(
            retryLimit: 3,
            initialInterval: TimeSpan.FromSeconds(1),
            intervalIncrement: TimeSpan.FromSeconds(2)));

        // Configure endpoint for payment authorization
        cfg.ReceiveEndpoint("payment-authorization-queue", e =>
        {
            e.ConfigureConsumer<AuthorizePaymentConsumer>(context);
            e.PrefetchCount = 10;
            e.ConcurrentMessageLimit = 5;
        });
    });
});

var host = builder.Build();
host.Run();
