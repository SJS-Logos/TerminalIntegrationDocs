using Microsoft.Extensions.DependencyInjection;
using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.Domain.Services;
using MyPaymentService.Core.SharedKernel;
using MyPaymentService.Infrastructure.InMemory;

namespace MyPaymentService.CliHost.Configuration;

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
