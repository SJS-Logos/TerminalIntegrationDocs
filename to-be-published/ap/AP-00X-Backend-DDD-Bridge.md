# AP-00X: Backend / DDD Bridge to AP Concepts

**AP:** 00X (informative)
**Status:** Draft
**Version:** 0.1
**Category:** Informative
**Author:** (bridge — derived from AP-000…AP-009 + `dotnet.instructions.md`)
**Depends on:** AP-001, AP-002, AP-003, AP-004, AP-005, AP-006, AP-007, AP-008, AP-009

> **Companion working notes:** `AP-DDD-Reconciliation-Notes.md` (per-rule analysis and defect taxonomy).

---

## 1. Purpose and status

This is a **non-normative onboarding bridge**, in the spirit of AP-000 (which bridges OTA
terminology into AP concepts). It bridges the vocabulary of the backend **DDD rule-set**
(`dotnet.instructions.md`, and the DDD Manifest under `04-Guides/Domain Driven Design/`) into
the normative **AP series (AP-000 … AP-009)**.

**AP-000 … AP-009 are normative for the whole organization** (embedded *and* backend). The DDD
rule-set is retained as an **informative implementation guide** for .NET services and MUST
conform to the APs. Where the two differ, **the AP series governs**; for dependency direction
specifically, **AP-006 §1.1 is the single source of truth.**

Why a bridge is needed: embedded developers meet the APs with technology-independence already
front of mind. Backend developers work in a stable .NET/EF/MassTransit stack and reason in DDD
terms, so certain dependency pitfalls are **invisible from inside that vocabulary**. This
document makes them visible without discarding the DDD idioms backend developers rely on.

---

## 2. Vocabulary map (DDD rule-set ? AP concept)

| DDD rule-set term | AP concept | Notes |
|---|---|---|
| Domain model (Entity / Value Object) | **Business State** in the **Domain** (AP-004 §4) | "Domain model" = Entity or Value Object **only** — not contracts, not read models. |
| Business rule / policy / domain service | **Business Behaviour** in the Domain (AP-004 §4) | Kept distinct from Business State. |
| DTO / Request / Response / Command / Event | **Contract Model** in the **Application** (AP-003 §5) | May carry transport/serialization concerns; holds no business rules. |
| Read model | **Read Model** in the **Application** (AP-009 §6) | Immutable, data-carrying; query side. |
| Repository **interface** | **Capability** in `Capabilities/` (AP-005) | Domain-owned abstraction — **not** infrastructure. |
| Repository **implementation** | **Adapter Implementation** (AP-007) | Technology-specific; lives outside the Core. |
| Application service | **Application** use case (AP-001, AP-004 §6) | Sole entry point to the Core. |
| "Infrastructure layer" | **Adapter Implementation** (AP-007) | Adapter is the AP name; `Infrastructure` is the legacy label. |
| API endpoint (`Api` host) | **Host Unit** (HTTP trigger) (AP-003) | `Api` is a valid deployment host kind. Translates transport
| Worker / consumer (`Worker` host) | **Host Unit** (message/schedule trigger) (AP-003) | `Worker` is a valid deployment host kind; same pattern, different trigger. |
| Domain event dispatch, message publishing | **Adapter** realizing a messaging **Capability** (AP-005/AP-007) | Not a Core/DbContext concern. |
| Unit of Work | Application-owned **transaction boundary**, realized by an Adapter (AP-004 §6.3) | UoW ? transaction (see §5). |

### 2.1 Glossary precision

* **Domain model = Entity or Value Object only.** Whenever a rule says "domain model", read it as
  "Entity or Value Object". It is **not** a Contract Model (DTO/Command/Event) and **not** a Read Model.
* **Domain model ? Contract model.** Contract Models are Application-layer data shapes that carry
  transport/serialization concerns and hold no business rules.
* **UoW ? transaction.** A Unit of Work is the application-level change-set owner; a transaction is
  the store's atomic commit. A UoW is *realized via* a transaction. EF fuses them (`DbContext` = UoW,
  `SaveChanges()` = transaction), which is the source of several confusions below.

---

## 3. The one picture: dependencies move toward business meaning

```text
   Host Unit (API / Worker)                     Adapter Implementations
   HTTP · message · schedule                    EF/SQL · HTTP client · broker
        ?  translate trigger                          ?  implements
        ?  ? Contract Model                            ?  Capability
        ?                                              ?
   ???????????????????????????? Core ???????????????????????????
   ?  Application  ???  Domain  ???  Capabilities  ???  SharedKernel ?
   ?  (use cases,       (Business    (domain-owned      (shared      ?
   ?   Contract Models,  State +      abstractions)      value types)?
   ?   Read Models)      Behaviour)                                  ?
   ??????????????????????????????????????????????????????????????????
```

Rules (AP-006 §4), restated for backend readers:

* **Host ? Application only.** An endpoint or consumer reaches business behaviour **exclusively**
  through the Application. **Host ? Domain is forbidden.**
* **Core ? nothing outside the Core.** Application/Domain/Capabilities/SharedKernel never reference
  a Host or an Adapter.
* **Adapter ? Capability (implements) and SharedKernel only.** An Adapter never depends on Domain
  internals; it realizes a Capability contract.

---

## 4. The three "invisible dependency" pitfalls

These are the backend-specific traps the DDD idioms hide. Each violates AP-006 §4.

### 4.1 Repository/UoW abstractions placed in `Domain/`

DDD guidance often defines `IRepository`/`IUnitOfWork` "in the domain layer" (R-1.4.2). Under the
APs these are **Capabilities** and belong in `Capabilities/`. Keeping them in `Domain/` blurs the
Domain's dependency surface.

**Fix:** interface ? `Capabilities/` (AP-005); implementation ? Adapter (AP-007).

### 4.2 `DbContext` implementing `IUnitOfWork` and dispatching domain events via MassTransit

A common pattern makes EF's `ApplicationDbContext` implement a domain `IUnitOfWork` **and** inject
MassTransit's mediator to publish domain events inside `SaveChangesAsync`. This puts
business-event choreography inside a **technology Adapter**, and makes the Adapter depend on the
Domain/Application — both forbidden (AP-006 §4).

**Fix:** the EF `DbContext` is a pure Adapter. The **Application** owns the transaction boundary
(AP-004 §6.3); domain-event dispatch is an Application/Adapter concern behind a Capability, not a
Core-embedded mediator.

### 4.2.1 Where MassTransit lives (acceptable, when abstracted)

MassTransit is **not forbidden** — it is fine, provided each of its two roles sits on the correct
side of the Core boundary and **no MassTransit-specific type reaches the Core**:

* **Inbound (consuming) ? a Host Unit.** A `MassTransitHost` (AP-003 §5.6) MAY use MassTransit types
  freely *inside itself* (`IConsumer<T>`, `ConsumeContext<T>`, endpoint/topology config). A Host Unit
  is transport-specific by definition; its job is to translate the message ? Contract Model ?
  Application use case (AP-003 §5.1).
* **Outbound (publishing) ? an Adapter.** When the Core needs to emit something, it depends only on a
  **domain-termed Capability** (e.g. `IEventPublisher.Publish(PaymentSettled evt)`) in `Capabilities/`
  (AP-005). A MassTransit-backed **Adapter** (AP-007) realizes it. The Adapter only translates and
  publishes — it does **not** decide *what* event to raise or *when* (that stays in the
  Domain/Application; AP-007 §4.2 forbids orchestration in Adapters).
* **Bus instantiation ? the Composition Unit.** The bus/transport is a **shared runtime** started
  **once** at the composition root (AP-003 §5.5). Host Units and the publishing Adapter are
  library-style units that only *register into* that bus (AP-003 §5.7.2); they do not each stand up
  their own instance. Multiple driving Host Units in one process share the single bus; separate
  processes each own their own — never two competing bus instances in one process.

**The invariant:** `IBus`, `IPublishEndpoint`, `ISendEndpoint`, `ConsumeContext`, `IConsumer<T>`,
and MassTransit routing/message attributes MUST NOT appear in `Domain/` or `Application/`. The test:
you could swap MassTransit for another broker or an in-memory fake by replacing only the Adapter
Unit, with zero Core edits. *(Reconciliation item 2c — resolved: acceptable behind a
Capability/Adapter; only Core-injected MassTransit types are the defect.)*

### 4.3 Host ? Domain shortcut for "simple" cases

`R-1.5.5` (app services only for multiple aggregates) + `R-2.2.4` (consumers may work with domain
models directly) jointly authorize **Host ? Domain**, bypassing the Application — the single most
important forbidden edge (AP-006 §4). It also invites business logic into the Host (contra R-2.2.2)
and forces re-plumbing when a "simple" use case grows.

**Fix:** one uniform path **Host ? Application ? Domain**, always. A thin one-line application
service is the stable boundary, not waste (AP-004 §6). *(Reconciliation item 3 — meeting.)*

---

## 5. Transactions and aggregate boundaries

### 5.1 One aggregate per transaction — and its one exception

`R-1.2.5` ("only one aggregate per transaction") is the right **default** but is stated as an
absolute. AP-004 §6.3 is more precise:

* Default: modify **one aggregate per transaction**.
* A multi-aggregate operation **MUST declare** whether it needs **atomic** or **eventual** consistency.
* Where atomic consistency is required, the **Application establishes** the transaction boundary and
  a **Capability + Adapter realize** it; the Domain Service expresses the operation but does not
  depend on a transaction mechanism.

The EF trap: `SaveChanges()` commits **all** tracked aggregates in one transaction, silently
enabling the multi-aggregate commit R-1.2.5 forbids. Keep the boundary explicit and
Application-owned. *(Reconciliation item 2.)*

### 5.2 Aggregate boundary decision ladder ("when are two roots really one?")

1. Is there a rule that is false the instant it is violated, even momentarily? **No ?** separate
   aggregates; coordinate by **ID** (R-1.2.4) + domain event.
2. **Yes ?** does the rule span both candidates? **No ?** each is its own aggregate.
3. **Yes ?** they are **one** aggregate; collapse the second "root" into a child under the surviving
   root (R-1.2.1, R-1.2.2).
4. Still tempted to modify two roots in one transaction? That is the tell (R-1.2.5) that step 1 was
   answered wrong — reclassify the rule as a true invariant.

An **ID reference** is the identity *token* only (`CustomerId`) — **not** a live object reference to
the other aggregate and **not** an embedded/shallow copy of its state.

---

## 6. Read models are not ORM-bound

A Read Model is a **pure data shape**, not a persistence artifact (AP-009 §6: no
transport/persistence/serialization concerns). The **projection** that reads the store (EF `.Select`,
a SQL view, Dapper) lives in a **read-side Adapter** (AP-009 §8), not in the Read Model.

* **Read Model (shape)** ? Application (AP-009 §6.1). It is the Query's return-type contract.
* **Projection (reading)** ? read-side Adapter (AP-009 §8). Keeps the Core from depending on the Adapter.

The intent behind "use projection" (R-1.3.4) is the AP-009 §8 guarantee: **produce read models
without materializing an Aggregate.** The specific idiom (deferred `IQueryable` projection) is a
.NET-profile detail — and note that *materialize-then-project* (`.ToList().Select(...)`) satisfies
the wording while defeating the intent.

---

## 7. Hosts: endpoints and workers are the same pattern

`R-2.1` (API) and `R-2.2` (Worker) are both **Host Units** (AP-003) that differ only by trigger.
`Api` and `Worker` are **not** retired — they remain valid **deployment host kinds**, each hosting
one or more Host Units:

| Trigger | Host | Deployment host | In DDD doc |
|---|---|---|---|
| HTTP request | endpoint | `Api` | R-2.1 |
| Message (command/event) | consumer | `Worker` | R-2.2 (partial) |
| **Schedule / poll** | scheduled worker | `Worker` | **missing** |
| gRPC / CLI | — | `Api` / `Worker` | missing |

Every Host has the same contract: **translate the trigger into a Contract Model, invoke an
Application use case, hold no business behaviour** (AP-003, AP-006 §4). The endpoint translates
**HTTP ? Contract Model ? Application**, never HTTP ? Domain. **Outbound** message publishing is an
**Adapter** realizing a messaging Capability — distinct from the Worker's inbound consuming role.

---

## 8. Testing: adopt the pyramid

`R-3.1` mandates only integration/end-to-end tests. AP-008 §5.1 prescribes the **test pyramid**:

* **Many unit tests** — business logic isolated, no DB, no deployment; edge cases, invariants,
  policies. Arrange by constructing aggregates in memory via domain methods (`Order.Create`,
  `order.AddItem`). Validate on **Outcome/state**.
* **Fewer adapter/integration tests** — an Adapter against real technology (Testcontainers). Arrange
  by seeding the **store directly** (repository/`DbContext`), not through the API.
* **Few end-to-end tests** — **good path only**; edge cases belong in unit tests.

**"No mocking" ? "must call infrastructure"** (R-3.1.4). Don't mock the **Core** (domain, application,
repositories). For **external** systems, substitute a **fake Adapter implementing the Capability**
(AP-005/AP-007) — the ports/adapters discipline is what makes tests fast without mocking the Core.
Real infrastructure is only needed in **adapter/integration** tests. *(Reconciliation item 5.)*

---

## 9. Rule-wording patterns (how to read the DDD rules under the APs)

The DDD rules encode good intent but recurringly bundle a **guarantee** with a **mechanism/technology**,
or state a mechanism without its guarantee. When reading a rule, separate the two:

* **Guarantee** (technology-neutral) ? normative, maps to an AP.
* **Mechanism** (C#/EF idiom) ? .NET implementation profile (AP-009 §1 makes this split explicit).

Recurring examples:

* **Guarantee fused with C# mechanism** — R-1.1.9, R-1.3.2, R-2.1.3, R-2.2.6 ("must be a `record`…") ?
  guarantee is *immutable / value-typed*; `record` is the mechanism.
* **Technology named in a prohibition** — R-1.1.10 ("EF Core attributes") ? guarantee is *no
  persistence/serialization coupling of any kind* (attributes, base classes, interfaces, proxies);
  EF is only an example.
* **Mechanism without its guarantee (open/floor)** — R-1.1.3 ("private setters + read-only
  collections") ? the guarantee is *state changes only through invariant-enforcing methods* (R-1.1.7);
  the stated mechanism is satisfiable while still exposing mutation.

For the full per-rule classification (11 categories) see `AP-DDD-Reconciliation-Notes.md` §7.

---

## 10. Reconciliation status

| Item | Topic | Disposition |
|---|---|---|
| 1 | Static factories (R-1.1.4) | **Resolved (proposed):** developer's choice — the AP series is silent on the creation mechanism. No AP change. |
| 2 | Cross-aggregate transactions (R-1.2.5, R-1.5.4) | **Change AP docs:** align to AP-004 §6.3 (default one aggregate; explicit atomic exception, Application-owned boundary). |
| 3 | Application-service scope (R-1.5.5, R-2.2.4) | **Meeting:** drop Host?Domain shortcut; uniform Host?Application?Domain. |
| 4 | Read models (R-1.3.x) | **Meeting:** placement/projection realization, per AP-009 §6.1/§8. |
| 5 | Testing strategy (R-3.1) | **Change AP docs / adopt AP-008:** add the test pyramid. |

Dependency-direction findings (§4.1–4.3) are governed by AP-006 §1.1 and correct the DDD guide
toward the APs.

---

## 11. What backend developers should do differently (checklist)

* Put repository/UoW **interfaces** in `Capabilities/`, **implementations** in Adapters — never in `Domain/`.
* Keep EF `DbContext` a pure Adapter: it does **not** implement domain interfaces and does **not**
  dispatch domain events.
* Route **every** use case through the **Application** — including single-aggregate ones.
* Make the transaction boundary **explicit and Application-owned**; don't rely on `SaveChanges()` to
  define scope.
* Keep Read Models pure; put the projection in a read-side Adapter; project **without materializing**
  an aggregate.
* Treat endpoints and workers as **Host Units** that translate a trigger into a Contract Model and
  call the Application.
* Test on the **pyramid**: unit (state/Outcome, no infra) ? adapter (real tech) ? e2e (good path);
  substitute fake **Adapters** rather than mocking the Core.
