using MassTransit;
using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Services;
using Logos.PaymentService.Domain.ValueObjects;
using Logos.PaymentService.Adapters;
using Logos.PaymentService.Messaging;

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

// Register MassTransit using centralized configuration from Messaging project
builder.Services.AddPaymentServiceMessaging(builder.Configuration);

var host = builder.Build();
host.Run();
