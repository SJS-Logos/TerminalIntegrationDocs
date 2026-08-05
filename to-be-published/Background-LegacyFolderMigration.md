# Background: Legacy Folder Migration

**Status:** Informative
**Audience:** Backend (.NET / C++) developers migrating existing services
**Related AP:** AP-002

---

# 1. Purpose

This document records the mapping from folder names used in services predating the AP series to their successors defined in AP-002. It is informative only and has no normative standing. Developers working on new services, or on embedded or non-.NET platforms, can disregard it.

---

# 2. Retired Names

The following folder names are retired. Each SHALL be replaced by its AP-002 successor in any service brought into compliance with the AP series.

| Legacy folder | Successor | Notes |
| --- | --- | --- |
| `Api` | Deployment Unit + dedicated Host Units | Replace the monolithic API folder with a deployment project (for example `ServiceName.Api`) that composes transport-specific Host Units (`HttpHost`, `MassTransitHost`, etc.). See AP-003. |
| `Core` | *(out of scope)* | Was a shared kernel of cross-cutting utilities — not the AP-001 Core. Cross-cutting shared utilities are out of scope for the AP series at this time. |
| `InfrastructureAbstractions` | `Capabilities/` folder within the Core Unit | Capabilities are now a peer folder alongside `Domain/`, `SharedKernel/`, and `Application/`. See AP-002 §5.1.3. |
| `Infrastructure` | Technology-specific infrastructure folders or Units | Split into focused implementations such as `Infrastructure.MsSql` rather than one monolithic `Infrastructure` folder. See AP-007. |
| `Worker` | Deployment Unit + dedicated Host Units | Replace the monolithic worker folder with a deployment project (for example `ServiceName.Worker`) that composes trigger/transport Host Units as needed. See AP-003. |

---

# 3. Migration Notes

The legacy `Worker`, `Api`, and `Infrastructure` folders were often monolithic, combining multiple technologies in one place. AP-002 and AP-003 replace that centralization by splitting inbound mechanisms into pure Host Units and composing them through dedicated Deployment Units, while keeping infrastructure concerns in focused technology-specific Units.

No mechanical rename is sufficient; each legacy folder should be reviewed against the AP-002 Unit definitions before migration.
