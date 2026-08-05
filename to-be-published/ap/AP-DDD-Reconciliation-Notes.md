# AP ? dotnet.instructions.md Reconciliation Notes (Working Draft)

**Status:** Working notes — not normative. Input for the backend/DDD bridge document and the reconciliation meeting.

**Purpose:** Capture, as they are discovered, the discrepancies between the normative AP series
(AP-000 … AP-009) and the backend DDD rule-set (`docs/dotnet.instructions.md`) and the
AiO DDD Manifest (`AiO-DeveloperDocumentation/docs/04-Guides/Domain Driven Design/`).

The intent of the merge is that **AP-000 … AP-009 become normative for the whole
organization** (embedded and backend). The backend material is retained as an *informative*
implementation guide that must conform to the APs, with a bridge that surfaces the
dependency pitfalls that are not obvious from inside the DDD vocabulary.

---

## 1. Reconciliation policy (from repo Copilot instructions)

- Items **1 (static factories)**, **3 (application-service scope)**, **4 (read models)** — require a meeting.
- Items **2 (cross-aggregate transactions)** and **5 (testing strategy)** — accommodate by changing the AP documentation.

Note: the UoW/MassTransit findings below turned out to be **dependency-direction** issues,
which AP-006 §1.1 declares itself the single source of truth for. Under that clause the
correction flows the other way (correct the DDD guide to match AP-006), so these are treated
as meeting items rather than mechanical AP edits.

---

## 2. Structural / dependency findings

| # | Finding | AP reference | Disposition |
|---|---------|--------------|-------------|
| 2a | `IRepository` / `IUnitOfWork` placed in `Domain/` | These are Capabilities ? belong in `Capabilities/` (AP-005); AP-006 §4 matrix | Correct DDD guide |
| 2b | EF `DbContext` implements domain `IUnitOfWork` **and** injects MassTransit `IMediator` to dispatch domain events | Adapter ? Domain / Application forbidden (AP-006 §4); transaction boundary is Application-owned (AP-004 §6.3) | Correct DDD guide |
| 2c | MassTransit used as the in-process mediator inside the Core | Technology-independence invariant (AP-001); Capability-as-Port (AP-005); Adapter realization (AP-007); bus started once at Composition root (AP-003 §5.5, §5.7.2) | **Resolved:** MassTransit is acceptable when confined to a `MassTransitHost` (inbound) and/or an Adapter realizing a messaging **Capability** (outbound), with the bus instantiated once at the Composition Unit. Only **Core-injected MassTransit types** (`IBus`, `IPublishEndpoint`, `ConsumeContext`, `IConsumer<T>`, routing attributes) are the defect. See bridge §4.2.1. |
| 2d | Legacy `Api` / `Core` / `Infrastructure` / `Worker` naming throughout | `Api` and `Worker` are **not** retired — they remain valid **deployment hosts** (Host Units, AP-003); `Infrastructure` remaps to Adapter (AP-007) | Vocabulary remap in bridge (Api/Worker retained as host kinds) |

---

## 2b. Glossary — precise vocabulary

- **Domain model** = **Entity** or **Value Object** only (the pure business types in the Domain).
  Whenever a rule says "domain model" (R-1.1.3, R-1.1.6, R-1.1.7, R-1.1.10, R-1.4.4, …), read it as
  "Entity or Value Object."
- **Domain model ? Contract model.** Contract models are DTOs / Requests / Responses (R-2.1.3) and
  Commands / Events (R-2.2.6); they live in the Application, carry transport/serialization concerns,
  and hold no business rules.
- **Read model** (R-1.3.x) is also an Application concern (query side), not a domain model.
- AP mapping: the AP series calls domain models **Business State** (Entities + Value Objects),
  distinct from **Business Behaviour** (rules, Policies, Domain Services) — AP-004 §4. Contract models
  are **Contract Models** in AP-003. Bridge glossary must map "domain model" ? "Business State types".

## 3. Read-model clarifications

- A **read model is not ORM-bound.** It is a pure data shape (AP-009 §6: MUST NOT carry
  transport/persistence/serialization concerns). EF/Dapper/SQL live in the read-side Adapter
  (AP-009 §8), not in the read model. Pattern 1's EF `.Select(...)` is one realization, not the
  definition. Good bridge example: EF projection vs. Dapper query producing the *same* read model.

---

## 4. Rule wording quality — "rules to re-anchor to AP guarantees"

Lens: does a rule state a **guarantee (closed / ceiling — names the only permitted path or a
prohibition)** or merely a **mechanism (open / floor — a necessary-not-sufficient ingredient
that can be satisfied while defeating its own intent)**?

| Rule | Kind | Issue | Re-anchor to |
|------|------|-------|--------------|
| R-1.1.1 | Closed (good) | Well-formed: drives identity equality and aggregate keying | AP-001 Entity definition |
| R-1.1.3 | **Open (floor)** | "private setters and read-only collections" is necessary-not-sufficient: satisfiable while still exposing public mutating methods, public mutable fields, or a mutable backing collection by reference ("top off with anything"). Partly redundant with R-1.1.7. | Reword as a closed encapsulation guarantee: state modifiable *only* through invariant-enforcing methods; no public setters, no public mutable fields, no mutable references to internal collections/children. Anchor to R-1.1.7 / AP-004 §4. |
| R-1.1.6 | Closed (good) | Well-formed prohibition (no data access / infrastructure logic in Entities/VOs). Behavioural sibling of R-1.1.10 (which forbids infrastructure *types*). Only weakness: "data access" / "infrastructure logic" are undefined terms of art | Keep force; add terminology gloss (DB/repo calls, network, files, messaging, framework services). Align to AP-004 §9 / AP-001 §6.3 |
| R-1.1.10 | Closed (good), but technology-coupled | Right shape (prohibition), but names "EF Core attributes" in normative text — reads as if EF is the presumed default already inside the domain ("EF invited in"). Also attribute-narrow: persistence coupling also enters via base classes, framework interfaces, navigation conventions, lazy-loading proxies, not just attributes | Restate technology-neutrally: no persistence/serialization technology at all (ORM attributes, base classes, interfaces, mappings; serialization attributes). Move "EF Core" to an illustrative .NET-profile example only. Align to AP-004 §7.3 / §9, AP-007 (ORM lives in the Adapter) |
| R-1.2.4 | Closed (good) | Well-formed prohibition (cross-aggregate reference by ID, not direct object reference). Clarification worth adding: an ID reference is the identity *token* only (`CustomerId`) — **not** a live object reference to the other aggregate, and **not** an embedded/shallow copy of its state | Keep force; add "ID token, not a copy, not a live reference" gloss. Align to AP-004 §6.3 |
| R-1.2.5 | Closed shape, but **over-constrained** | "Only one aggregate per transaction" states an absolute where the architecture actually allows a conditional. Transaction = one atomic DB commit (`SaveChanges`). Correctly forbids using a shared transaction to paper over a wrong boundary, but drops the legitimate case where a multi-aggregate operation genuinely requires atomic consistency | **Relax to match AP-004 §6.3**: default one aggregate per transaction; where an operation explicitly requires atomic multi-aggregate consistency, the **Application** owns the transaction boundary and declares atomic vs. eventual. This is reconciliation **item 2**. |
| R-1.2.6 | **Non-normative aspiration in a MUST clause** | "Keep aggregates small" has no criterion (undefined "small"/"large"), so it is not reviewer-actionable despite the doc requiring rule-ID citations. It also duplicates the effect of R-1.2.3 / R-1.2.5: aggregate size is *determined* by the true-invariant/consistency boundary, so "small" is a consequence, not an independent rule. Residual real content is only a tie-breaker heuristic (smaller = less contention) in genuinely ambiguous designs | Demote to guidance prose, or delete as subsumed by R-1.2.3 / R-1.2.5. AP series deliberately carries no standalone "keep small" MUST — size emerges from the invariant boundary (AP-004 §6.3, AP-001 "group by reason to change") |
| R-1.2.7 | **Derived (theorem, not axiom)** | "All writes through aggregate roots" is *entailed*, not independent: (a) from R-1.2.1 + R-1.1.7 + R-1.4.1 (roots control members, changes via methods, repositories only for roots ? no other write door); and (b) from dependency direction (AP-006 §4) — a persistence Adapter may not reference Domain internals, so it *cannot* bypass a root. Redundant MUST ? drift risk + obscures load-bearing rules | Demote to rationale prose. Re-anchor with the sharper AP statement: the root does not write; a persistence **Adapter** writes whole aggregates addressed through the root, Domain performs no persistence (AP-004 §9, AP-006 §4, AP-007) |
| R-1.3.2 | **Fused: guarantee + platform mechanism** | Bundles a technology-neutral guarantee (*immutable, data-carrying*) with a C# mechanism (*`record` + primary constructor*). The mechanism is one of several ways to be immutable (`init`-only class, readonly struct…) and is meaningless outside C#. As written it would falsely fail a perfectly immutable read model that used `init`-only properties | Split: keep "immutable + data-carrying" as the normative guarantee (= AP-009 §6); move "`record` with primary constructor" to the **.NET profile** as guidance (AP-009 §1 explicitly assigns C# mechanics to the profile) |
| R-1.3.3 | Closed (good) | Correct (read models in Application, not Domain) — aligns with AP-009 §6.1. But the *rationale* "domain doesn't touch infrastructure" only proves **not-in-Domain**; it does not pick Application over the read-side Adapter. Sharper justification: the **Core must not depend on the Adapter** (AP-006 §4), so the Query's return-type contract (the read model) must live in the Core/Application while the **projection** that reads infrastructure lives in the read-side Adapter (AP-009 §8). The two halves together prevent a Core?Adapter inversion | Keep force; correct rationale and note the read-model(Application)/projection(Adapter) pairing (AP-009 §6.1 + §8, AP-006 §4) |
| R-1.3.4 | **Fused (ORM idiom) + Open (floor)** | "Use projection to map directly instead of loading full domain models" hard-codes an EF/`IQueryable` idiom and names a *mechanism*, not the *guarantee*. "Projection" conflates two things: the neutral concept (select a subset of fields into a shape) and the ORM behaviour (deferred `IQueryable.Select` translated into SQL so only projected columns are fetched). Because it only names the mechanism, "materialize-then-project" (`.ToList().Select(...)`) satisfies the letter while over-fetching the whole graph. It also states the tactic as an absolute with no cost model (doesn't apply when most of the entity is needed, for complex joins, or small sets) | Re-anchor to the AP-009 §8 guarantee: *produce read models **without materializing an Aggregate***. Move the specific idiom (deferred `IQueryable` `.Select`, or a SQL view / Dapper) to the **.NET profile** with the over-fetch cost model as guidance (AP-009 §1, §8) |
| R-1.3.5 | Closed (good) | Clean prohibition; maps to AP-009 §6 (read model MUST NOT depend on Domain Aggregates/Entities, and is shaped for a read purpose, not mirroring an aggregate). Forbids object references to (a) domain models — reintroduces write-side coupling and mutability — and (b) other read models — cycles + uncontrolled loading. Only issue: "navigation property" is EF jargon for a neutral idea | Keep force; add terminology gloss: *no object references to entities or other read models; carry copied values and ID tokens instead* (mild technology-coupling in the wording only). Align AP-009 §6 |

### Cross-cutting defect — "guarantee fused with C# mechanism"
The same de-fuse applies to a family of rules that hard-code a C# form inside a MUST. Keep the guarantee normative; move the mechanism to the .NET profile:

- **R-1.1.9** — "Value Objects must be of the record type" ? guarantee: *immutable, value-equality*; mechanism: `record`.
- **R-1.1.10** — "EF Core attributes" (names a technology to avoid) ? guarantee: *no persistence/serialization coupling*; example only in profile.
- **R-1.3.2** — read model `record` + primary ctor ? guarantee: *immutable*; mechanism in profile.
- **R-2.1.3 / R-2.2.6** — DTOs/Commands/Events "must be of type Record with a primary constructor" ? guarantee: *immutable Contract Models*; mechanism in profile.

AP boundary: AP-009 §1 states C#-specific mechanics belong to the .NET implementation profile, not the normative AP.
| R-1.4.4 | Closed (good), imprecise wording | Correct rule (repositories return domain models); only its *sibling* placement rules (R-1.4.2 domain / R-1.4.3 infrastructure) use retired vocabulary. Wording weak spot: **"DTO" names a shape, not an ownership/layer** — a read model *is* structurally a DTO, so "not DTOs" fails to express the real prohibition (no *read-side/transport* shape, and no persistence entity). Also "repository = infrastructure for an aggregate" conflates the two halves: the repository **contract** is a **Capability** (Core, not infrastructure); only the **implementation** is an Adapter | Restate positively: a write repository's contract returns the **Aggregate Root** (Business State: root entity + owned entities/value objects) or a simple control type (existence/count/void); MUST NOT return a persistence entity (R-1.1.10 leak) or a read-side/transport model (R-1.3.1, wrong side). Remap placement to Capabilities (AP-005, contract) / Adapter (AP-007, impl). Align AP-004 (Business State), AP-009 (query side) |
| R-1.4.5 | Closed (good), **partly derived** | "No `IQueryable` from repositories" leaks *two* things: (1) a **technology type** in the Core contract — this half is a **dependency-direction consequence** (AP-006 §4: a Capability may not name an ORM type), a local shadow like R-1.2.7; and (2) **deferred, composable query control** — the caller could keep composing `.Where/.Include`, execute against the live connection, and author queries outside the repository. Half (2) is an *independent* encapsulation guarantee that dependency direction alone does not give. "`IQueryable`" is EF/LINQ jargon for "a live, composable query handle" | Keep force. Note the split: type-leak half entailed by AP-006 §4; query-control half is the independent teeth, matching **AP-009 §8** ("MUST NOT expose a technology-specific query abstraction / live query object beyond its boundary"). Add terminology gloss for non-EF stacks |
| R-1.5.4 | **Underspecified + latent contradiction with R-1.2.5** | "App services handle transactions and unit of work" assigns a responsibility with no scope constraint. Definitional issue: **UoW ? transaction** — UoW is the application-level *change-set owner* (tracks dirty objects, flushes together); transaction is the store's *atomic commit*; a UoW is *realized via* a transaction. EF fuses them (`DbContext` = UoW, `SaveChanges()` = transaction), so the UoW's default behaviour (commit *all* dirty aggregates in one transaction) **actively enables** the multi-aggregate transaction R-1.2.5 forbids. The doc gives no mechanism to reconcile the two — latent contradiction | Split the three concerns AP-004 §6.3 separates: (1) **Application owns/establishes** the boundary; (2) a **Capability + Adapter realizes** the transaction (UoW/`SaveChanges` lives *outside* the Core — also fixes §2b); (3) **scope** = one aggregate by default (R-1.2.5), widened only by explicit atomic declaration (AP-004 §6.3). Same **item 2** cluster. Add UoW?transaction definitional note |
| R-1.5.5 (+ R-2.2.4) | **Rule pair authorizing a dependency-direction violation** | "App services only when orchestrating *multiple* domain models" — the word *only* forbids an app service for single-aggregate cases, so *something else* must call the Domain directly. R-2.2.4 names it: "consumers **may work with domain models directly** for simple use cases." Result: **Host ? Domain** for simple cases, bypassing the Application. Violates **AP-006 §4** (Host/Deployment ? Domain = *forbidden, use Application*) and **AP-004 §6** (Application invokes a single Domain entry point per use case). Also invites business logic to leak into the Host (contra R-2.2.2, open-door like R-1.1.3), and forces re-plumbing when a use case grows from one to many aggregates. **Direction of each half:** R-1.5.5 *passively removes* the boundary (no app service for simple cases); R-2.2.4 *actively grants* the crossing (consumer may touch domain). **Internal contradiction:** R-2.2.4 permits exactly the business-logic-in-Host that R-2.2.2 forbids, and "simple" is undefined/unstable (floor, like R-1.2.6/R-1.1.3) | Drop the "only when multiple" carve-out and the "consumers may work with domain directly" allowance; use one uniform path **Host ? Application ? Domain** (a thin one-line app service for single-aggregate cases is the stable boundary, not waste). **Reconciliation item 3** — meeting decision, not a unilateral edit |

### Section-level finding — R-2.1 (API Design) is a misplaced Host concern

R-2.1 describes the **HTTP API surface** — an inbound transport, i.e. a **Host Unit / incoming Implementation (AP-003)** — but sits inside a document framed as DDD/domain requirements (`applyTo: **/*.cs`). Three structural problems:

1. **Misplaced / layer switch with no signpost.** Transport-adapter rules are mixed into domain-modelling rules; AP-002/AP-003 keep Host concerns in a separate unit outside the Core.
2. **Prohibition-only; positive contract missing.** R-2.1 says what endpoints receive/return (R-2.1.1/.2) and must not do (R-2.1.4 no domain exposure; R-2.1.6 no writes), but **never states the endpoint's actual job**: translate the transport payload into a **Contract Model** and invoke an **Application use case**. AP-003 states this positively.
3. **Application (the sole callee) never mentioned.** The pipeline `HTTP ? endpoint ? Application use case ? response` is missing its middle term, so R-2.1.4/.6 read as arbitrary "don'ts" instead of consequences of "the endpoint calls the Application, not the Domain."

Precision: endpoints translate **HTTP ? Contract Model ? Application**, *never* HTTP ? Domain (endpoint reaching Domain directly is the same R-1.5.5/R-2.2.4 boundary violation). R-2.1.3 also carries the fused-`record` defect (guarantee: immutable Contract Model; mechanism ? .NET profile). Align to AP-003 (Host translates transport ? Contract Models, no business behaviour), AP-006 §4 (Host ? Application only).

### Section-level finding — R-2.2 (Worker Design) never states the Worker's reason to exist + coverage gap

Same class as R-2.1 (misplaced Host concern, missing positive contract), plus a **trigger coverage gap**. R-2.2 describes the Worker *mechanically* (has consumers, consumers handle commands/events) but never states **why a Worker exists** or **what triggers it**.

A Worker is simply the **Host Unit for non-HTTP triggers** (AP-003 lists "HTTP, message broker, CLI, gRPC, **scheduler, poller**" — all Host Units with the same contract: translate trigger ? Contract Model ? invoke Application use case, no business behaviour). The doc only acknowledges **one** trigger type and omits others:

1. **Message-driven** (inbound commands/events) — covered by R-2.2.1/.3.
2. **Time-driven (scheduler/poller)** — **missing**. A timer job has *no message*, so it doesn't fit "consumers handle commands/events"; developers have nowhere to put a scheduled sweep or will fake it as a "consumer." This is a *primary* reason a Worker exists vs. an API.
3. **Outbound publishing** ("sending domain messages") — **unaddressed**. In AP terms, *publishing* is a messaging **Adapter** realizing a Capability, triggered by the Application/Domain — distinct from the Worker's *inbound consuming* role. The doc conflates/omits this.

Because the "Worker = Host driven by messages or a schedule" framing is absent, R-2.2.4/.5/.7 float free (missing middle term = the Application, same as R-2.1). Already-logged sub-defects: R-2.2.4+R-2.2.5 (dependency-direction violation, item 3), R-2.2.6 (fused-`record`). Align to AP-003 (Host trigger taxonomy + translation contract), AP-006 §4 (Host ? Application only), AP-005/AP-007 (outbound publishing = Adapter realizing a messaging Capability).

### Section-level finding — R-3.1 (Testing) is single-tier; no test pyramid (item 5)

R-3.1 has **only** an Integration Tests subsection and mandates "all added code covered by integration tests" (R-3.1.1) "end-to-end including database" (R-3.1.2) — i.e. it puts *all* coverage at the slowest, most brittle tier. Incomplete, not wrong. **Reconciliation item 5** (accommodate by adopting the fuller AP doc — AP-008 already *is* the pyramid).

AP-008 §4/§5.1 supplies the missing tiers: **many unit tests** (business logic isolated, no DB, edge cases/invariants) ? **fewer component/adapter tests** (adapter vs. real technology) ? **few end-to-end** (good path only; "SHALL NOT exhaustively test edge cases").

Two practical gaps the thinness creates (user questions):

1. **Data injection — inject at the layer under test, not through the API.** API is for *acting*, not *arranging*.
   - Unit: construct aggregates in memory via their own domain methods (`Order.Create`, `order.AddItem`) — possible only because Domain is pure (R-1.1.6).
   - Adapter/integration: seed **directly into the store** via repository/`DbContext` in arrange, real DB (Testcontainers, R-3.1.3) — use the adapter to arrange, not HTTP.
   - E2E: arrange via API only for the good-path act; prefer DB seeding for arrange.

2. **"No mocking" ? "must call infrastructure" — two different axes.** R-3.1.4 (don't mock domain/app/repos) is right, but because R-3.1 only has integration tests it *looks* like "no mocks" forces "real infrastructure." The pyramid separates them:
   - Unit: validate by asserting on the aggregate's **Outcome/state** — no infrastructure, no mocks (nothing external in the test).
   - Adapter: validate against **real technology** (Testcontainers) — no mocks, real infra, adapter only.
   - External systems (HTTP APIs, brokers): substitute a **fake/stub Adapter implementing the Capability** (AP-005/AP-007) — not a mock of the Core. The ports/adapters discipline is what makes this possible.
   Honest rule: *don't mock the Core; substitute Adapters (fakes) for external technology; use real infrastructure only in adapter/integration tests.* Align to AP-008 §4, §5.1.

### Section-level finding — R-4 (Code Style) is correct but miscategorized

R-4 (official .NET conventions, meaningful names, short single-responsibility methods) is **correct and uncontroversial**, but these are **universal craftsmanship norms**, not architectural requirements — true of any C# codebase regardless of DDD. New category: **correct-but-miscategorized**.

Problem is placement, not content. Like R-1.2.6 (heuristic in a MUST) but from the opposite end: R-4 is a *style norm* in a MUST, **not objectively enforceable** ("meaningful", "short", "single responsibility" have no bright line), yet the preamble demands rule-ID citation in review. Mixing hard, checkable architectural constraints (R-1.4.5, R-2.1.4, dependency direction) with soft, subjective style preferences **dilutes "mandatory"** and weakens the authority of the enforceable rules by association.

AP framing: the APs constrain *architecture* (boundaries, dependency direction, decision ownership, testability); code style is a **.NET profile / tooling** concern (analyzers, `.editorconfig`, `dotnet format`), the kind AP-009 §1 assigns to the implementation profile. Disposition: **relocate** R-4 out of the mandatory set into a code-style guidance doc, and **automate** where possible so "compliance" is mechanical, not a subjective review argument. No content change.

---

## 7. Defect taxonomy (summary across the whole rule-set)

Categories discovered while walking R-1…R-4:

1. **Well-formed (closed/good)** — R-1.1.1, R-1.1.6, R-1.2.4, R-1.3.3, R-1.3.5, R-1.4.4, R-1.4.5.
2. **Under-constrained (open/floor)** — necessary-not-sufficient, satisfiable while defeating intent: R-1.1.3, R-1.3.4 (also fused), "simple"/"small" undefined predicates (R-1.5.5/R-2.2.4, R-1.2.6).
3. **Technology-coupled / fused with C# mechanism** — guarantee welded to a C# form/vendor in a MUST: R-1.1.9, R-1.1.10, R-1.3.2, R-1.3.4, R-2.1.3, R-2.2.6. ? split guarantee (normative) from mechanism (.NET profile), per AP-009 §1.
4. **Over-constrained** — forbids a case the architecture permits under explicit conditions: R-1.2.5. ? relax to AP-004 §6.3.
5. **Non-normative aspiration in a MUST** — no criterion: R-1.2.6. ? guidance.
6. **Derived / entailed by dependency direction** — a local shadow of AP-006 §4: R-1.2.7, R-1.4.5 (type-leak half). ? rationale prose.
7. **Underspecified / latent internal contradiction** — R-1.5.4 (vs R-1.2.5; UoW?transaction).
8. **Rule pair authorizing a dependency-direction violation** — R-1.5.5 + R-2.2.4 (Host?Domain; also self-contradicts R-2.2.2). ? item 3.
9. **Misplaced Host concern (section-level)** — R-2.1, R-2.2 belong in AP-003; prohibition-only, missing positive contract + Application middle term; R-2.2 also has a trigger coverage gap (no scheduler/poller, no outbound publishing).
10. **Single-tier testing (section-level)** — R-3.1 lacks the pyramid ? item 5, adopt AP-008.
11. **Correct-but-miscategorized (section-level)** — R-4 style norms in a mandatory set ? relocate + automate.
(To be extended as the walkthrough continues.)

---

## 5. Aggregate boundary guidance (decision ladder for the bridge)

1. Is there a rule that is false the instant it is violated, even momentarily? No ? separate aggregates; coordinate by ID (R-1.2.4) + event.
2. Yes ? does the rule span both candidates? No ? each is its own aggregate.
3. Yes ? they are one aggregate; collapse the second "root" into a child entity under the surviving root (R-1.2.1, R-1.2.2).
4. Still tempted to modify two roots in one transaction? That is the tell (R-1.2.5) that step 1 was answered wrong — reclassify the rule as a true invariant.

AP-004 §6.3 adds the forcing function the DDD rules lack: a multi-aggregate operation MUST
explicitly declare atomic vs. eventual consistency, and coordination stays root-to-root.
This only bites if the transaction boundary is explicit (Application-owned), which is the same
reason the UoW/MassTransit correction (§2b) matters.

---

## 6. Open meeting items

Two dispositions feed this list: **meeting** items need a group decision before any AP/guide edit;
**AP-doc** items are already agreed in direction and only need the edit made. Resolved items are
retained here for the record with their disposition marked.

### Needs a decision (meeting)

- **Item 1 — static factories (R-1.1.4).** Proposed resolution: *developer's choice* — the AP series
  is silent on the creation mechanism, so no AP change; the DDD rule relaxes from MUST to guidance.
  **Decision sought:** confirm "developer's choice" and demote R-1.1.4 to guidance.
- **Item 3 — application-service scope (R-1.5.5 + R-2.2.4).** These jointly authorize **Host ? Domain**
  for "simple" cases, violating AP-006 §4. Proposed resolution: drop the "only when multiple aggregates"
  carve-out and the "consumers may work with domain directly" allowance; adopt one uniform path
  **Host ? Application ? Domain** (a thin one-line application service is the boundary, not waste).
  **Decision sought:** approve the uniform path and the wording change to R-1.5.5/R-2.2.4.
- **Item 4 — read-model placement/projection realization (R-1.3.x).** Direction is clear from AP-009
  (read model = Application-layer contract §6.1; projection = read-side Adapter §8), but placement,
  the record/primary-ctor mechanism (R-1.3.2), and the projection idiom (R-1.3.4) need sign-off.
  **Decision sought:** confirm read-model-in-Application / projection-in-Adapter split and move the
  C# mechanics to the .NET profile.

### Accommodate by changing the AP docs (agreed direction)

- **Item 2 — cross-aggregate transactions (R-1.2.5, R-1.5.4).** Align to AP-004 §6.3: default one
  aggregate per transaction; a multi-aggregate operation MUST declare atomic vs. eventual consistency,
  with the **Application** owning the boundary and a **Capability + Adapter** realizing it. Also record
  the **UoW ? transaction** definitional note. **Action:** edit the AP/guide accordingly.
- **Item 5 — testing strategy (R-3.1).** Adopt the AP-008 test pyramid (many unit ? fewer
  component/adapter ? few end-to-end, good-path only), plus the "arrange at the layer under test" and
  "don't mock the Core; substitute fake Adapters" clarifications. **Action:** replace the single-tier
  R-3.1 with a reference to AP-008.

### Resolved (recorded for the meeting minutes)

- **§2a / §2b — repository/UoW placement and `DbContext`/`IMediator` coupling.** Dependency-direction
  defects governed by AP-006 §1.1: interfaces are Capabilities (AP-005), implementations are Adapters
  (AP-007), and the transaction boundary is Application-owned (AP-004 §6.3). *Correct the DDD guide.*
- **§2c — MassTransit "in-process mediator".** Resolved: MassTransit is acceptable when confined to a
  `MassTransitHost` (inbound) and/or an Adapter realizing a messaging Capability (outbound), with the
  bus started once at the Composition Unit (AP-003 §5.5/§5.7.2). Only **Core-injected MassTransit
  types** are the defect. See bridge §4.2.1.
- **§2d — `Api` / `Worker` naming.** Not retired; both remain valid deployment host kinds (Host Units,
  AP-003). Only `Infrastructure` remaps to Adapter (AP-007). *Vocabulary remap only.*
