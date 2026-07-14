using Logos.Payment.Core.Application.UseCases;
using Logos.Payment.Core.Capabilities;
using Logos.Payment.Core.Domain.Services;
using Logos.Payment.Core.SharedKernel;
using Logos.Payment.Infrastructure.InMemory;

namespace Logos.Payment.HttpHost.Configuration;

public static class ServiceConfiguration
{
    public static IServiceCollection AddPaymentServices(this IServiceCollection services)
    {
        services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
        services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

        services.AddSingleton<PaymentAuthorizationService>(sp =>
        {
            var fraudService = sp.GetRequiredService<IFraudDetectionService>();
            var maxAmount = new Money(10000m, "USD");
            return new PaymentAuthorizationService(fraudService, maxAmount);
        });

        services.AddScoped<AuthorizePaymentUseCase>();
        services.AddScoped<GetPaymentUseCase>();

        return services;
    }
}
