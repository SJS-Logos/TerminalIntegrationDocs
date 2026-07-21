namespace MyPaymentService.Hosting.Abstractions;

/// <summary>
/// Optional composition contract an Adapter MAY expose when it owns a
/// versioned or stateful backing store that must reach readiness before any
/// Capability operation runs (AP-007 §8.2). Stateless Adapters do not
/// implement it. The host discovers and runs all registered initializers.
/// </summary>
public interface IAdapterInitializer
{
    Task InitializeAsync(CancellationToken cancellationToken = default);
}
