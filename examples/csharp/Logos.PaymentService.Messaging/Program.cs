using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Services;
using Logos.PaymentService.Domain.ValueObjects;
using Logos.PaymentService.Adapters;
using Logos.PaymentService.Messaging;

var builder = Host.CreateApplicationBuilder(args);

// Register Domain Services
builder.Services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
builder.Services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

builder.Services.AddSingleton<PaymentAuthorizationService>(sp =>
{
    var fraudService = sp.GetRequiredService<IFraudDetectionService>();
    var maxAmount = new Money(10000m, "USD");
    return new PaymentAuthorizationService(fraudService, maxAmount);
});

// Register Use Cases
builder.Services.AddScoped<AuthorizePaymentUseCase>();

// Configure MassTransit with RabbitMQ
builder.Services.AddPaymentServiceMessaging(builder.Configuration);

var host = builder.Build();
host.Run();
