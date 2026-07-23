# AP-000: Introduction to AP Concepts Through OTA

**Document type:** Informative introduction  
**Audience:** Engineers familiar with OTA who are onboarding to the AP series  
**Status:** Draft

---

# 1. Purpose

This document introduces the AP architecture model (DDD + Ports and Adapters) using familiar structures from the legacy OTA codebase.

It is intentionally **non-normative**. Normative requirements remain in AP-001 through AP-009.

---

# 2. Why this document exists

Many embedded engineers already understand OTA operationally. The AP series describes similar architectural boundaries using modern terms.

This document provides a translation layer between:

- OTA naming and structure
- AP concepts (`Application`, `Domain`, `Capabilities`, `Host`, `Adapter`)

---

# 3. OTA-to-AP Concept Mapping

## 3.1 High-level mapping

| OTA area | Typical role in OTA | Closest AP concept |
|---|---|---|
| `src/OtaApi` | Entry API used by application-facing callers | `Application` surface + incoming contract boundary |
| `src/Ota` | Payment state machine and business behavior | `Domain` |
| `src/HalOtaGeneric` (base classes) | Abstract hardware/OS-facing interfaces (`*BaseClass_T`) | `Capabilities` (ports) |
| `src/HalOtaValina`, `src/HalOtaCTOS` | Platform/device-specific concrete implementations | `Adapter Implementations` |
| `src/OtaEventHandler` | Callback/event integration interface with external host app | Incoming/outgoing boundary at host integration edge |
| `src/OtaUtil`, `src/Tapa`, `src/UtilC` | Utility, protocol, conversion helpers | Mixed: shared technical support; not a direct AP core concept |

## 3.2 Important nuance

This is a **conceptual** mapping, not a one-to-one folder migration recipe. OTA predates the AP model by many years, so concerns are sometimes co-located.

---

# 4. Evidence from OTA structure

Representative examples in OTA that align with AP concepts:

- `src/include/Ota/Ota/Ota.h` defines a large domain state machine (`OTA_STATE_LIST`) and transaction-flow behavior. This corresponds to AP Domain behavior ownership.
- `src/include/Ota/HalOtaGeneric/FileSystem_Class.h` and `Time_Class.h` define abstract base interfaces for external capabilities (filesystem/time), matching Ports/Capabilities.
- `src/HalOtaValina/*` and `src/HalOtaCTOS/*` provide technology/device-specific realizations, matching AP Adapter Implementations.
- `src/include/Ota/OtaApi/OtaApi.h` exposes application-facing operations (`InitiatePurchase`, `InitiateRefund`, etc.), aligning with an application/use-case entry boundary.
- `src/include/Ota/OtaEventHandler/IOtaEventHandler.h` defines integration callbacks/events at the boundary to surrounding systems.

---

# 5. Reading OTA with DDD + Ports/Adapters terms

## 5.1 Domain (business meaning)

In AP terms, the Domain owns business decisions and state transitions.

In OTA, much of that behavior lives in `src/Ota` (for example payment flow/state transitions). The AP model would preserve that ownership while isolating it from transport/device technology.

## 5.2 Capabilities (ports)

In AP terms, Capabilities are domain-owned abstractions for external needs.

In OTA, many `HalOtaGeneric` base classes behave this way (time, filesystem, card readers, pinpad, etc.): they are interfaces the business behavior depends on, without hardcoding one device/platform implementation.

## 5.3 Adapters

In AP terms, adapters are concrete technology bindings.

In OTA, `HalOtaValina` and `HalOtaCTOS` provide exactly that: concrete platform/device implementations behind the abstract capability interfaces.

## 5.4 Application / Host boundary

In AP terms, `Application` orchestrates use cases, while host-side incoming implementations handle transport/runtime concerns.

In OTA, `OtaApi` and event-handler integration (`IOtaAPIEventHandler_T`) together form much of this boundary. The AP model separates these concerns more explicitly, but the intent is already familiar.

---

# 6. Shared Kernel in OTA

The AP model defines a dedicated `SharedKernel` for stable, reusable business datatypes.

In OTA, equivalent datatypes are present but distributed (for example message and amount-related types in `OtaApi`/`Ota` headers). A key AP improvement is to gather shared business datatypes into an explicit, disciplined shared-kernel area.

---

# 7. Practical onboarding checklist for OTA engineers

When moving from OTA mental model to AP model:

1. Identify business decision logic first (`Domain`).
2. Identify abstract external needs next (`Capabilities` / ports).
3. Move platform specifics behind those interfaces (`Adapters`).
4. Keep incoming transport/runtime code in host boundaries (`Host Units`).
5. Keep shared business datatypes explicit and stable (`SharedKernel`).

---

# 8. Relationship to normative AP documents

Use this introduction as a vocabulary bridge only. Normative requirements are in:

- AP-001 (Architectural Principles)
- AP-002 (Service Structure)
- AP-003 (Incoming Implementations)
- AP-004 (Domain)
- AP-005 (Capabilities)
- AP-006 (Dependency Rules)
- AP-007 (Adapters)
- AP-008 (Testing)
- AP-009 (Read Models and Queries)

If this introduction and any AP conflict, the AP document governs.
