# RFC: Session-Oriented Process Coordination (SOPC) — Core Model

**Version:** 0.2  
**Status:** Draft  
**Author:** SJS  
**Created:** 2026-08-03  

---

# 1. Introduction

This RFC defines the **core model** for coordinating long-running, stateful interactions across UI, device, and distributed service boundaries.

The model generalizes Saga-style behavior without introducing a mandatory central orchestrator. Coordination is driven by append-only session events and local participant logic.

This document establishes the basics: the Step, its message lifecycle, the Session Log, and the local Session data model. The composition of Steps into an end-to-end flow — the **Session Script** — is specified separately in **SOPC-002**.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** in this document are to be interpreted as described in BCP 14 (RFC 2119 and RFC 8174) when, and only when, they appear in all capitals, as shown here.

---

# 3. Motivation

Distributed flows often become tightly coupled when one central runtime controls all step transitions.

SOPC aims to preserve local autonomy while keeping end-to-end flow semantics explicit, auditable, and recoverable.

A Step is defined not by internal implementation but by the **messages it subscribes to** and the **messages it produces**. This keeps Steps autonomous and independently deployable.

This model is intended to replace a central coordinator — a single, ever-growing state machine that owns every transition — with declared coordination; see the non-normative migration guidance in SOPC-002 §11.

---

# 4. Scope

In scope for this Core Model document:

- A runtime Session model for stateful execution.  
- The Step message lifecycle: activation, completion, failure, compensation, and data query.  
- An append-only, versioned Session Log as the shared coordination surface.  
- A storage-agnostic model for locally persisted Session data.  
- Retention and persistence requirements for that data.  
- The term Health Entity, as a free-standing, identified entity separate from the flow model, whose state changes via messages consumed by a health agent.  
- Deployment profiles spanning single-process and distributed realizations of the same abstraction.  

Out of scope (see SOPC-002):

- The Session Script (static flow definition), its versioning, and deployment distribution.  
- Transition and coordination semantics between Steps.  
- Declaration of global data-element identifiers per Step.  

Out of scope (entirely):

- Global ACID guarantees across independent services.  
- A mandated broker, log, or serialization technology.  
- UI rendering details or transport-specific API contracts.  

---

# 5. Core Concepts

## 5.1 Session (Runtime)

A Session represents one execution instance of a Session Script.

The runtime identity of a Session is minimal. A Session SHALL include only:

- `SessionId` — the identity of this concrete execution instance.  
- A reference to the governing Session Script, sufficient to resolve it: `(ScriptId, ScriptVersion)`.  

All other properties a Session appears to "have" — its steps, their message names, and their input/output parameters — are not stored on the Session. They are **derived** by resolving the Session Script (definition) against the Session Log (history). Values used only to verify or validate the script at deployment time are a deployment concern and are not published to the application runtime.

A Session MUST be reconstructable from its Session Log together with its Session Script.

## 5.2 Step Context (Self-Location)

When a Step is activated as part of a Session, it MUST be able to locate itself within the flow using only the identifiers carried on the `Activation`: the `SessionId`, the `(ScriptId, ScriptVersion)`, and its own `StepId` (step name).

Given these, a Step SHALL resolve, from the Session Script:

- The message names bound to its actions (`Activation`, `Rollback`, `Done`, `Failed`).  
- Its input data-element identifiers and its output data-element identifiers.  

This resolution is backed by the Session Script interface methods `MessageBindings(StepId)`, `Inputs(StepId)`, and `Outputs(StepId)` defined in SOPC-002 §4.2.

A Step MUST NOT depend on any Session-level state beyond this resolution; everything it needs is a function of `(Script, StepId)` for definition and `(SessionId, StepId)` for data. Current position in the flow is derived from the Session Log, not stored on the Session.

## 5.3 Step

A Step is an atomic unit of interaction.

A Step is defined by the messages it consumes and produces.

A Step SHOULD be short-lived: it exists to carry a single activation (or compensation) to a terminal outcome and then ceases. A Step MUST NOT be relied upon as a long-lived owner of hardware or interface health, because it is not guaranteed to be resident between activations. Long-lived hardware and interface health is modeled separately as Health Entities (Section 5.7).

A Step implementation MUST be idempotent for every message it consumes.

## 5.4 Step Messages

A Step participates through **five** message types:

1. `Activation` — inbound. Requests the Step to begin execution for a Session.  
2. `Rollback` — inbound. Requests the Step to compensate a previously completed execution for a Session.  
3. `Done` — outbound. Signals a terminal successful outcome.  
4. `Failed` — outbound. Signals a terminal failure outcome, with metadata sufficient for compensation or operator handling.  
5. `Query` — inbound request / outbound response. Requests the contents of the global data object(s) a Step holds for a given Session and Step identifier (see Section 7.3).  

A Step SHOULD subscribe to `Activation`, `Rollback`, and `Query` for the Steps it implements.

A Step SHALL publish a terminal outcome (`Done` or `Failed`) for each `Activation` and for each `Rollback` it processes.

## 5.5 Step Lifecycle

A Step has **two entry points** and **one terminal alphabet**.

Forward lifecycle:

1. `Activation` received.  
2. Execution.  
3. Terminal outcome: `Done` or `Failed`.  

Compensation lifecycle (explicit):

1. `Rollback` received for a Step that previously reached `Done`.  
2. Compensating execution.  
3. Terminal outcome: `Done` or `Failed`.  

Compensation is itself an activation of a compensating action and therefore MUST terminate in `Done` or `Failed`. A `Failed` outcome for a `Rollback` MUST be treated as a compensation failure and escalated per Section 9.

A `Rollback` for a Step that never reached `Done` MUST be a no-op that still emits `Done` (idempotent compensation).

## 5.6 Participant

A Participant is a service that hosts one or more Step handlers.

A Participant SHALL publish terminal outcome events (`Done` or `Failed`) for each activated or rolled-back Step.

A Participant SHALL respond to `Query` requests for Session data it persists locally.

## 5.7 Health Entities

Because Steps are short-lived (Section 5.3), they cannot act as continuous health monitors for the hardware or interfaces they manipulate. Long-lived health is therefore modeled as a set of **Health Entities**: free-standing, addressable entities decoupled from the Step lifecycle and from any single Session, rather than being bundled into a device handler.

A Health Entity SHALL have:

- A **unique, stable identifier** that names the monitored component (for example, a device, interface, or subsystem) and is independent of any `SessionId`.  
- A current **health state** that MAY change over the entity's lifetime.  

A Health Entity is not mutated by direct field assignment. Instead, its state changes are driven by **health state-change messages** addressed to the entity's identifier. Each message declares an observed or requested state transition for that entity; the entity's current state is the result of applying these messages in order. This mirrors the message-driven model used for Steps: producers emit state-change messages, and the entity's state is derived from them rather than shared as mutable memory.

Health state-change messages MAY be **consumed by a health agent (or health manager)** — a long-lived role, separate from the flow, that aggregates the state of many Health Entities, exposes a consolidated view, and MAY react to degraded or failed states (for example, by raising alarms or disabling a component). The health agent is a consumer of health state changes, not a Session Coordinator, and MUST NOT drive Session transitions.

The flow model is unrelated to Health Entities. A Step MAY emit a health state-change message or read a Health Entity's current state, but health observation is orthogonal to coordination and never drives Session transitions. The internal state model of a Health Entity, the vocabulary of its state-change messages, and how the health agent aggregates or polls them are out of scope for this RFC.

## 5.8 Session Initiation

A Session does not exist until it is initiated. Initiation is the act of minting a new `SessionId` for a given `(ScriptId, ScriptVersion)` and recording the first event in a new Session Log, from which the first Step is activated.

Initiation is driven by a **trigger** that is external to any running Session. A trigger is typically:

- a user action (for example, a card inserted or a button pressed), or  
- a timer or scheduled event, or  
- another observable condition in the environment.  

The mechanism is a **start signal and a poll**. A Session Script MAY declare a start signal; emitting that signal activates an initiation poll that evaluates the trigger condition and decides whether a new Session should begin:

1. A start signal is emitted (by a device, UI, scheduler, or other source).  
2. An **initiator** evaluates the trigger condition for the candidate `(ScriptId, ScriptVersion)`.  
3. If the condition holds, the initiator mints a new `SessionId` and records the first event, activating the first Step.  
4. If the condition does not hold, no Session is created and no state is retained.  

The initiator is a distinct role from a Step: it runs before any Session exists and therefore cannot rely on `SessionId` or Session data. It is a purely runtime role and is not part of the Session Script interface. Typically the initiator is the environment's **idle loop** — the resting state that continuously watches for a trigger and is the natural home for the "may a new Session begin?" decision. Evaluation of the trigger condition MUST be idempotent with respect to a single start signal, so that a repeated or redelivered signal does not create duplicate Sessions for the same triggering event.

Note (non-normative): idempotency is commonly realized by edge detection on the trigger — for example debouncing a hardware input, read-and-clear consumption of a UI button press, or a one-shot flag — so that a level-held or redelivered signal initiates at most one Session.

The declaration of a Session's start signal and trigger binding is a Session Script concern (see SOPC-002).

# 6. Session Log

The Session Log is the authoritative append-only event history for a Session.

## 6.1 Required Properties

Each event in the Session Log MUST include:

- A globally unique event identifier.  
- A session identifier.  
- A step identifier.  
- A timestamp.  
- A schema version.  
- A payload.  
- Correlation and causation identifiers where applicable.  

The Session Log MUST be append-only.

The Session Log MUST support replay to rebuild Session state deterministically.

## 6.2 Schema Evolution

Event schemas MUST be versioned.

Schema changes SHOULD preserve backward compatibility for existing readers.

Breaking schema changes MUST be introduced with a new schema version and explicit migration strategy.

---

# 7. Session Data Model

## 7.1 State Contract

Session state MUST be represented as typed, immutable value objects.

The wire and at-rest encoding of Session data (a form keyed by globally unique data-element identifiers, per SOPC-002 §9) is an implementation detail and MUST NOT be the normative state contract. Adapters MAY translate between such encodings and the typed value objects, but the value objects remain authoritative.

A free-form dictionary MAY be used only as an internal adapter detail and MUST NOT be the normative Session state contract. Such a dictionary is a transitional adapter, for example when migrating a legacy shared structure; the target design is copy-by-value into declared data-elements (§7.5).

Each Step MAY contribute additional value objects to Session state, but MUST NOT mutate prior events.

## 7.2 Local Persistence

A Step MAY persist the Session data it produces locally, keyed by `(SessionId, StepId)`.

Where a Step persists data locally, the Session Log remains authoritative: local storage is a materialized projection and MUST be reconstructable by replaying the Session Log. Loss of a local store MUST NOT cause permanent loss of Session data.

## 7.3 Data Query

A Step SHALL support a `Query` (Section 5.4) that returns the global data object(s) it holds for a given `(SessionId, StepId)`.

Collecting the full data set for a Session is performed by querying each participating Step, or by querying a projection built from the Session Log. The result identifies each data object by its global data-element identifier (defined in the Session Script, see SOPC-002).

## 7.4 Retention and Persistence Requirements

Local Session data retention SHALL be bounded by an explicit policy. This RFC defines the requirement; concrete durations are deployment configuration.

The following are **open for decision** and MUST be resolved before ratification:

- Minimum retention window for locally persisted Session data after a Session reaches a terminal state.  
- Whether retention is driven by time, by Session terminal state, or by explicit purge command.  
- Durability requirement for local data (in-memory, crash-durable, or replicated) relative to the authority of the Session Log.  
- Behavior of a `Query` against data that has been purged locally but is still derivable from the Session Log.  

## 7.5 External Entities and Copy-by-Value

Session data MUST NOT hold a live reference or pointer to an external entity (for example, a customer, agreement, or device record) that is owned and mutated outside the Session.

Instead, a Step that needs data from an external entity SHALL resolve that entity during its execution and **copy the needed fields as values into Session data**, published as the Step's output data-elements. From that point the Session carries a self-contained snapshot: subsequent Steps consume the copied values, not the external source.

This preserves the immutability contract of §7.1 and the replay guarantee of §6: because the snapshot is captured in a `Done` event at the moment of resolution, replaying the Session Log reproduces the same values regardless of later changes to the external entity. A Step that requires a fresher value MUST re-resolve the entity and publish a new data-element, rather than aliasing the original source.

Until resolved, a Participant SHOULD retain local Session data at least until the Session reaches a terminal state and all compensations have completed.

---

# 8. Query and Derivation

The platform MUST support querying by `SessionId`.

The platform MUST support querying by Step within a Session.

The platform MUST support querying a Step for its global data objects (Section 7.3).

The platform SHOULD support filtering by event type and time range.

Derived business artifacts (for example, receipt or ticket views) MAY be produced from projections over the Session Log.

---

# 9. Reliability and Failure Handling

Delivery semantics depend on the deployment profile (Section 10). Where delivery is not exactly-once, delivery MAY be at-least-once and Consumers MUST deduplicate by event identifier. In a single-process profile where delivery is exactly-once, deduplication MAY be a no-op.

Retries SHOULD use bounded backoff policies.

If a Step cannot complete after retries, the Participant MUST emit a `Failed` event and include failure metadata sufficient for compensation or operator handling.

If a `Rollback` cannot complete after retries, the Participant MUST emit a `Failed` event for the compensation and escalate for operator handling.

---

# 10. Deployment Profiles

SOPC defines a single abstraction — one Step contract, one Session Script — that is realized across a spectrum of deployments. The abstraction is invariant; the mechanisms that realize it differ by profile. Conformance to SOPC does **not** require a message broker or a distributed infrastructure.

Two reference profiles anchor the spectrum:

## 10.1 Single-Process Profile

One process runs the flow, typically one Session at a time (for example, a payment terminal).

- The four Step messages (`Activation`, `Rollback`, `Done`, `Failed`) and `Query` MAY be realized as in-process calls or an in-memory queue rather than broker messages.  
- Delivery is effectively exactly-once; deduplication (Section 9) MAY be a no-op.  
- The Session Coordinator (SOPC-002 §7.1) is centralized and generic.  
- `ScriptId`/`ScriptVersion` checks (SOPC-002 §6) are trivially satisfied by the single deployment, but still apply.  
- The Session Log (Section 6) remains valuable for crash recovery, for example after a power cycle.  

A single-process realization is a first-class, conformant SOPC deployment. Features intended for distribution are latent — present in the model but trivially satisfied — and MUST NOT be treated as prerequisites.

## 10.2 Distributed Profile

Steps are hosted by independent services, potentially running many concurrent Sessions.

- The Step messages are real messages over a broker; delivery is typically at-least-once, so deduplication by event identifier is mandatory (Section 9).  
- Compensation (`Rollback`) is real distributed-Saga behaviour.  
- The Session Coordinator MAY be distributed (Participants self-activate) or a centralized generic driver (SOPC-002 §7.1).  
- Concurrent Sessions make `SessionId` partitioning and ordered delivery live concerns.  
- Copy-by-value (Section 7.5) is a necessity, since a live reference across a service boundary is not possible.  

The additional coordination challenges of the distributed profile — notably ownership of the Session Log across service boundaries — are the subject of ongoing work (Section 13).

## 10.3 Profile Invariance

The Step contract, the Session Script, the Session Log semantics, and copy-by-value are identical across profiles. Moving a flow from single-process to distributed changes the realization of message delivery and the placement of the Session Coordinator, but MUST NOT change the Session Script or the Step contracts.

## 10.4 Heterogeneous-Language Boundary (Non-Normative)

A deployment commonly spans **several implementation languages** at once — for example C# microservices, C++ embedded firmware, and Java / Kotlin on Android — that must all coordinate within the same flow. Wherever a Step message (`Activation`, `Rollback`, `Done`, `Failed`, `Query`) crosses from one language runtime to another, it is realized as a serialized message (for example JSON over a broker) rather than an in-process call.

Such a boundary is a **transport-adapter concern only** and is deliberately not modeled by SOPC. Each language runtime hosts a translational endpoint that serializes and deserializes Step messages and maps them field-by-field onto its local contract — it contains no coordination or business logic. Existing repository examples demonstrate this shape on two sides of the boundary already: a C# consumer (`docs/examples/AP-003-MassTransit-Consumer-CSharp.md`) and its C++ equivalent with a purely translational `MessageMapping` (`docs/examples/AP-003-RabbitMQ-Consumer-Cpp.md`); a Java / Kotlin (Android) endpoint is a further adapter of the same shape.

The interoperability surface between languages is therefore the **message schema, not shared code or shared types**. Because each runtime owns its own local contract behind a translational adapter, adding a language (for example Kotlin on Android) adds one more adapter over the same message vocabulary and requires no change to any other Participant.

Two consequences follow from the model already defined above:

- Because the boundary only serializes the five Step messages, it introduces no new Step, Session, or Session Log semantics. A Participant hosting a Step handler is an ordinary Participant (Section 5.6) regardless of whether it is written in C#, C++, Java, or Kotlin; the language is invisible to the Session Script.  
- Copy-by-value (Section 7.5) is already mandatory whenever a message crosses a process boundary, so a live cross-language reference is never required; the snapshot carried on the message is sufficient for the receiving runtime to act.  

A deployment that bridges, say, an in-process C# coordinator to local C++ device handlers and to an Android (Kotlin) UI remains a conformant Single-Process (Section 10.1) or Distributed (Section 10.2) realization depending on whether the bridged messages are in-memory or broker-carried. The number of languages involved adds no requirement to this RFC.

---

# 11. Security and Compliance

Sensitive payload fields MUST be classified and protected at rest and in transit.

Access to Session, Session Log, and locally persisted Session data MUST be scoped by least privilege.

Redaction rules SHOULD be defined for derived views used outside trusted operational boundaries.

---

# 12. Relationship to Sagas

A Saga is a specialization of SOPC where the flow concerns distributed business transactions.

SOPC generalizes this model to any stateful interaction, including UI wizards, terminal flows, provisioning workflows, and device update pipelines.

---

# 13. Open Questions

Genuine spec-level questions:

- Durability class for locally persisted Session data (Section 7.4), in particular whether a power-cycling terminal requires crash-durable local persistence.  
- Staleness policy for copied external-entity snapshots (Section 7.5): when a Step MUST re-resolve rather than reuse a prior copy.  
- Ownership of the Session Log across service boundaries in the distributed profile (Section 10.2).  

Deferred to implementation, not this RFC:

- Retention window durations for locally persisted Session data (deployment configuration, Section 7.4).  
- Whether a `Query` collects data by scatter-gather across Steps or from a central Session Log projection.  

---

# 14. References

[1] S. Bradner, "Key words for use in RFCs to Indicate Requirement Levels," BCP 14, RFC 2119, March 1997.

[2] B. Leiba, "Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words," BCP 14, RFC 8174, May 2017.

[3] H. Garcia-Molina and K. Salem, "Sagas," *ACM SIGMOD Record*, vol. 16, no. 3, pp. 249-259, 1987. https://doi.org/10.1145/38713.38742

[4] SOPC-002, "Session-Oriented Process Coordination — Session Script & Coordination."
