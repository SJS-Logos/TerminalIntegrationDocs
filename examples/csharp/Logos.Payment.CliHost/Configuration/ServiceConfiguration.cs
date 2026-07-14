using Microsoft.Extensions.DependencyInjection;
using Logos.Payment.Core.Application.UseCases;
using Logos.Payment.Core.Capabilities;
using Logos.Payment.Core.Domain.Services;
using Logos.Payment.Core.SharedKernel;
using Logos.Payment.Infrastructure.InMemory;

namespace Logos.Payment.CliHost.Configuration;

public static class ServiceConfiguration
{
    public static void ConfigureServices(IServiceCollection services)
    {
        // Register infrastructure (capability implementations)
        services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
        services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

        // Register domain service with configuration
        services.AddSingleton<PaymentAuthorizationService>(sp =>
        {
            var fraudService = sp.GetRequiredService<IFraudDetectionService>();
            var maxAmount = new Money(10000m, "USD");
            return new PaymentAuthorizationService(fraudService, maxAmount);
        });

        // Register use cases
        services.AddScoped<AuthorizePaymentUseCase>();
        services.AddScoped<GetPaymentUseCase>();
    }
}
