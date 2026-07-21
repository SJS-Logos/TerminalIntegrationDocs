using MyPaymentService.Hosting.Abstractions;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Migrations;

namespace MyPaymentService.Infrastructure.EntityFramework;

/// <summary>
/// Idempotent initialization step that brings the SQLite backing store to the
/// required schema version before any Capability operation runs (AP-007 §8.2).
/// The migration rules are internal to the Adapter (AP-007 §8.1).
/// </summary>
public sealed class EntityFrameworkMigrator : IAdapterInitializer
{
    private readonly PaymentDbContext _context;
    private readonly PersistenceOptions _options;

    public EntityFrameworkMigrator(PaymentDbContext context, PersistenceOptions options)
    {
        _context = context;
        _options = options;
    }

    public async Task InitializeAsync(CancellationToken cancellationToken = default)
    {
        // Idempotent: applying against an already-current store is a no-op (§8.2).
        var pending = await _context.Database.GetPendingMigrationsAsync(cancellationToken);
        if (!pending.Any())
        {
            return;
        }

        // TargetVersion is Adapter configuration, never a Contract/Message parameter (§8.1).
        if (_options.TargetVersion is { } target)
        {
            await _context.GetService<IMigrator>().MigrateAsync(target, cancellationToken: cancellationToken);
        }
        else
        {
            await _context.Database.MigrateAsync(cancellationToken);
        }
    }
}
