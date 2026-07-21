using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.Domain.Services;
using MyPaymentService.Core.SharedKernel;
using MyPaymentService.Infrastructure.InMemory;

namespace MyPaymentService.HttpHost.Configuration;

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
