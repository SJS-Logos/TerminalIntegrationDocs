namespace Logos.Payment.Service.Infrastructure.EntityFramework;

/// <summary>
/// Configuration for the Entity Framework persistence Adapter.
/// The target migration version is Adapter configuration, never a
/// Capability Contract or Message parameter (AP-007 §8.1).
/// </summary>
public sealed class PersistenceOptions
{
    public string ConnectionString { get; init; } = "Data Source=payments.db";

    /// <summary>Optional pinned target migration; null migrates to latest.</summary>
    public string? TargetVersion { get; init; }
}
