# Adapter Migration Integration Example - C#

**Version:** 0.1  
**Status:** Draft  
**Applies to:** AP-007 (Adapter Implementations) Section 8  
**Builds on:** [AP-002 Implementation - C#](AP-002-Implementation-CSharp.md)

---

## 1. Introduction

This example demonstrates how **schema migration** is integrated when a Persistence Adapter is realized with Entity Framework (AP-007 Section 9.1), following the split required by AP-007 Section 8:

- The **migration behaviour lives in the Adapter** (Technology Scope). - Section 8.1
- The Adapter exposes an **idempotent initialization step** that ensures schema readiness before any operation runs. - Section 8.2
- The **host wiring invokes** that step at startup; it does **not** contain migration logic. - Section 8.3

> **Compilable Example Available**  
> A fully compilable and runnable version lives in the C# solution:  
> - Adapter: `MyPaymentService.Infrastructure.EntityFramework/`  
> - Host with startup migration: `MyPaymentService.EntityFrameworkHost/`  
>
> See the [README](../../examples/csharp/README.md) for build and run instructions.

---

## 2. Key Points

- Migration is a **Technology Scope** concern owned by the Adapter, not a Use Case.
- The target schema version is **Adapter configuration**, never a Capability Contract or Message parameter.
- Initialization is **idempotent** and runs **before** the host serves traffic.
- Failure to reach the required version is **fatal to startup** by default.

---

## 3. Where Each Responsibility Lives

The Capability the core depends on is unchanged by migration - it stays in domain terms (AP-005), with no versioning or initialization concept leaking onto it (AP-007 Section 8.1).

| Responsibility | File |
| --- | --- |
| Capability (unchanged by migration) | MyPaymentService.Core/Capabilities/IPaymentRepository.cs |
| Technology-scope persistence model (SQL row) | MyPaymentService.Infrastructure.EntityFramework/PaymentRow.cs |
| DbContext (confined to the Adapter Unit) | MyPaymentService.Infrastructure.EntityFramework/PaymentDbContext.cs |
| Repository translation (domain to SQL) | MyPaymentService.Infrastructure.EntityFramework/EntityFrameworkPaymentRepository.cs |
| Initialization contract (shared, opt-in) | MyPaymentService.Hosting.Abstractions/IAdapterInitializer.cs |
| Migration routine (idempotent MigrateAsync) | MyPaymentService.Infrastructure.EntityFramework/EntityFrameworkMigrator.cs |
| Target version as Adapter configuration | MyPaymentService.Infrastructure.EntityFramework/PersistenceOptions.cs |
| Adapter registration | MyPaymentService.Infrastructure.EntityFramework/InfrastructureRegistration.cs |
| Generic init loop (runs all registered initializers) | MyPaymentService.EntityFrameworkHost/Configuration/ServiceConfiguration.cs |
| Generated EF migration | MyPaymentService.Infrastructure.EntityFramework/Migrations/ |
| Host wiring (invokes, does not contain, migration) | MyPaymentService.EntityFrameworkHost/Configuration/ServiceConfiguration.cs |
| Entry point: readiness before traffic | MyPaymentService.EntityFrameworkHost/Program.cs |

---

## 4. How Migration Runs at Startup

The host entry point (EntityFrameworkHost/Program.cs) awaits the Adapter's initialization step **before** mapping any endpoint:

```csharp
var app = builder.Build();

// Schema readiness precondition (AP-007 Section 8.2): migration runs before the
// host serves traffic. An unhandled failure here aborts startup.
await app.Services.InitializeInfrastructureAsync();
```

InitializeInfrastructureAsync (in the host's ServiceConfiguration) enumerates every registered IAdapterInitializer and runs each one; stateless Adapters register none. The migration logic itself stays inside EntityFrameworkMigrator (AP-003 Section 5.3, AP-007 Section 8.3).

---

## 5. Optional: Migration as a Separate One-Shot Host

Where migration is deployed as its own step (for example a Kubernetes init container or job), a thin host composes **only** the Adapter and runs its initialization step - no migration logic of its own (AP-007 Section 8.3; AP-003 Section 5.7):

```csharp
var builder = Host.CreateApplicationBuilder(args);
builder.Services.AddEntityFrameworkPersistence(new PersistenceOptions
{
    ConnectionString = builder.Configuration.GetConnectionString("Payments")!
});

using var host = builder.Build();

// Run readiness, then exit. Non-zero exit on failure keeps deployment gated.
await host.Services.InitializeInfrastructureAsync();
```

---

## 6. Compliance Checklist

| Requirement | Where satisfied |
| --- | --- |
| Migration owned by the Adapter (8.1) | EntityFrameworkMigrator |
| Target version is Adapter configuration (8.1) | PersistenceOptions.TargetVersion |
| Capability meaning unchanged (8.1) | IPaymentRepository untouched |
| Idempotent initialization step (8.2) | GetPendingMigrationsAsync guard |
| Readiness before any operation (8.2) | Program.cs awaits before endpoints |
| Failure fatal to startup (8.2) | Unhandled exception aborts startup |
| Host invokes, does not contain, migration (8.3) | ServiceConfiguration.InitializeInfrastructureAsync (generic loop) |
| One-shot wrapper composes the Adapter only (8.3) | Section 5 pattern |
