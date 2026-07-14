using Logos.Payment.Service.Core.Application.UseCases;
using Logos.Payment.Service.Core.Capabilities;
using Logos.Payment.Service.Core.Domain.Services;
using Logos.Payment.Service.Core.SharedKernel;
using Logos.Payment.Service.Infrastructure.InMemory;
using Logos.Payment.Service.MasstransitHost.Consumers;
using MassTransit;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;

namespace Logos.Payment.Service.MasstransitHost.Configuration;

public static class ServiceConfiguration
{
    public static IServiceCollection AddPaymentServices(
        this IServiceCollection services,
        IConfiguration configuration)
    {
        // Register Domain services
        services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
        services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

        services.AddSingleton<PaymentAuthorizationService>(sp =>
        {
            var fraudService = sp.GetRequiredService<IFraudDetectionService>();
            var maxAmount = new Money(10000m, "USD");
            return new PaymentAuthorizationService(fraudService, maxAmount);
        });

        // Register Use Cases
        services.AddScoped<AuthorizePaymentUseCase>();

        // Configure MassTransit with RabbitMQ
        services.AddMassTransit(x =>
        {
            x.AddConsumer<AuthorizePaymentConsumer>();

            x.UsingRabbitMq((context, cfg) =>
            {
                cfg.Host(configuration["RabbitMQ:Host"] ?? "localhost",
                    configuration["RabbitMQ:VirtualHost"] ?? "/", h =>
                {
                    h.Username(configuration["RabbitMQ:Username"] ?? "guest");
                    h.Password(configuration["RabbitMQ:Password"] ?? "guest");
                });

                cfg.ConfigureEndpoints(context);
            });
        });

        return services;
    }
}
