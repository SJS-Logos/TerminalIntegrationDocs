using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.Domain.Services;
using MyPaymentService.Core.SharedKernel;
using MyPaymentService.Hosting.Abstractions;
using MyPaymentService.Infrastructure.EntityFramework;
using MyPaymentService.Infrastructure.InMemory;

namespace MyPaymentService.EntityFrameworkHost.Configuration;

public static class ServiceConfiguration
{
    public static IServiceCollection AddPaymentServices(
        this IServiceCollection services, string connectionString)
    {
        // Persistence Capability realized by the Entity Framework Adapter (AP-007 §9.1).
        services.AddEntityFrameworkPersistence(new PersistenceOptions
        {
            ConnectionString = connectionString
        });

        // Fraud detection stays a separate Capability, realized in-memory here.
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

    /// <summary>
    /// Runs every registered Adapter initialization step (schema readiness, etc.)
    /// before the host serves traffic. Generic over any Adapter that opts in by
    /// registering an <see cref="IAdapterInitializer"/>; stateless Adapters register
    /// none and cost nothing (AP-003 §5.3, AP-007 §8.2-§8.3).
    /// </summary>
    public static async Task InitializeInfrastructureAsync(this IServiceProvider services)
    {
        using var scope = services.CreateScope();
        foreach (var initializer in scope.ServiceProvider.GetServices<IAdapterInitializer>())
        {
            await initializer.InitializeAsync();
        }
    }
}
