# Copilot Instructions

## Project Guidelines
- For .NET services, the org's default technologies are Entity Framework (persistence) and MassTransit (messaging). MassTransit is pinned to v8, the last release under the open-source Apache 2.0 license; later releases moved to a commercial license. Corrects an earlier note that said legacy v7.
- Prefer the simplest possible project structure to minimize structural complexity. Evaluate AP-003 choices accordingly.
- Design for multiple Core units, with Host units potentially integrating capabilities from more than one Core, and Deployment units composing/merging multiple services. A Host must be implemented for one specific Core, while a Deployment must be able to deploy many Hosts and Cores. Core and pure Host units should be co-located in the same top-level area (often NuGet-packaged), with Deployment projects kept as local composition/runtime entry points.
- Use top-level service folders named like `Payment.Service` and `Loyalty.Service` for better readability.

## DDD/AP Reconciliation
- Items 1 (static factories), 3 (application-service scope), and 4 (read models) require a meeting.
- Items 2 (cross-aggregate transactions) and 5 (testing strategy) should be accommodated by changing the AP documentation.