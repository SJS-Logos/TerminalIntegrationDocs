# Terminal Integration Docs Overview

Technical specifications for integration between an embedded terminal and backend.

---

## SOPC RFC Index

| RFC | Title | Version | Status |
|-----|-------|---------|--------|
| [SOPC-001](sopc/SOPC-001.md) | Session-Oriented Process Coordination (SOPC) — Core Model | 0.2 | Draft |
| [SOPC-002](sopc/SOPC-002.md) | Session-Oriented Process Coordination (SOPC) — Session Script & Coordination | 0.2 | Draft |

## Architecture Pattern (AP) Index

| AP | Title | Category | Version | Status |
|----|-------|----------|---------|--------|
| [AP-000](ap/AP-000-OTA-Introduction.md) | Introduction to AP Concepts Through OTA | Informative | - | Draft |
| [AP-001](ap/AP-001.md) | Architectural Principles | Foundational | 0.1 | Draft |
| [AP-002](ap/AP-002.md) | Service Structure | Structural | 0.1 | Draft |
| [AP-003](ap/AP-003.md) | Incoming Implementations | Structural | 0.1 | Draft |
| [AP-004](ap/AP-004.md) | Domain | Structural | 0.1 | Draft |
| [AP-005](ap/AP-005.md) | Domain Capabilities | Structural | 0.1 | Draft |
| [AP-006](ap/AP-006.md) | Dependency Rules | Structural | 0.1 | Draft |
| [AP-007](ap/AP-007.md) | Adapter Implementations | Structural | 0.1 | Draft |
| [AP-008](ap/AP-008.md) | Testing Strategy | Structural | 0.1 | Draft |
| [AP-009](ap/AP-009.md) | Read Models and Queries | Structural | 0.1 | Draft |

AP-000 is an informative onboarding bridge from OTA terminology to AP concepts. The normative AP series builds cumulatively: AP-001 establishes technology-independent invariants, AP-002 turns them into a concrete service structure, and AP-003 through AP-009 refine each area (incoming implementations, domain, capabilities, dependency rules, adapters, testing, and read models/queries). Each AP declares the APs it depends on.

### AP Abstracts

**AP-000 — Introduction to AP Concepts Through OTA (Informative)**  
A non-normative onboarding bridge for engineers familiar with OTA. It maps legacy OTA structures to AP concepts (Domain, Application, Capabilities, Adapters, and Host boundaries) to ease transition into the AP vocabulary.

**AP-001 — Architectural Principles (Foundational)**  
Defines the architectural invariants governing all services: a technology-independent structure promoting maintainability, testability, and portability. It specifies logical boundaries (Application, Domain, Shared Kernel, Capabilities) without prescribing technologies, deployment models, or folder names.

**AP-002 — Service Structure (Structural)**  
Translates the logical invariants of AP-001 into a concrete service structure: how logical areas become compilation Units, how those Units may reference one another, and how they are named. Establishes that structure describes logical boundaries, not deployment topology, and retires the legacy `Api`/`Core`/`Infrastructure`/`Worker` folder names.

**AP-003 — Incoming Implementations (Structural)**  
Specifies how inbound transports and triggers (HTTP, message broker, CLI, gRPC, scheduler, poller) are structured as Host Units. Host Units translate between transport payloads and the Application's Contract Models and contain no business behaviour.

**AP-004 — Domain (Structural)**  
Expands the Domain principles into implementation guidance: how entities, value objects, policies, domain services, and business outcomes are organized within the Domain Unit, and where the Shared Kernel resides.

**AP-005 — Domain Capabilities (Structural)**  
Specifies how the Domain's Capabilities — domain-owned abstractions describing what the Domain requires from external systems — are expressed. Capabilities live alongside the Domain and are realized later by Adapter Implementations (AP-007).

**AP-006 — Dependency Rules (Structural)**  
The single source of truth for dependency direction across the series. Consolidates the allowed and forbidden dependencies between incoming implementations, the Domain, Capabilities, and Adapters into an enforceable matrix, restating the invariant that dependencies move toward business meaning.

**AP-007 — Adapter Implementations (Structural)**  
Specifies Adapter Implementations — the concrete, technology-specific realizations of Domain Capabilities. An Adapter's sole responsibility is translation between the Domain scope and a technology scope (HTTP, SQL, message formats, external APIs); it contains no business decisions and is replaceable.

**AP-008 — Testing Strategy (Structural)**  
Expands the testing structure into a full strategy: the categories of tests, what each verifies, and how test Units respect the dependency rules — applying the invariant that business behaviour is verifiable without deployment.

**AP-009 — Read Models and Queries (Structural)**  
Specifies the query side of a service: immutable Read Models, read-only Queries, and projection through read-side adapters, while preserving the write-side invariant that business decisions remain in the Domain.
