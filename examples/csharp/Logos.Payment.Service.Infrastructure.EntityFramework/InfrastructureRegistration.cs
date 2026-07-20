using Logos.Payment.Service.Core.Capabilities;
using Logos.Payment.Service.Hosting.Abstractions;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;

namespace Logos.Payment.Service.Infrastructure.EntityFramework;

/// <summary>
/// Registers the Entity Framework persistence Adapter. Technology initialization
/// is co-located with the Adapter (AP-007 §5.2).
/// </summary>
public static class InfrastructureRegistration
{
    public static IServiceCollection AddEntityFrameworkPersistence(
        this IServiceCollection services, PersistenceOptions options)
    {
        services.AddSingleton(options);
        services.AddDbContext<PaymentDbContext>(o => o.UseSqlite(options.ConnectionString));

        services.AddScoped<IPaymentRepository, EntityFrameworkPaymentRepository>();
        services.AddScoped<IAdapterInitializer, EntityFrameworkMigrator>();

        return services;
    }
}
