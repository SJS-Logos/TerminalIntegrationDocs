# RFC: Session-Oriented Process Coordination (SOPC) — Session Script & Coordination

**Version:** 0.2  
**Status:** Draft  
**Author:** SJS  
**Created:** 2026-08-03  
**Depends on:** SOPC-001 (Core Model)

---

# 1. Introduction

This RFC specifies the **coordination layer** of SOPC: the **Session Script** that composes autonomous Steps (defined in SOPC-001) into an end-to-end flow, together with its versioning, deployment distribution, transition semantics, and global data-element mapping.

SOPC-001 defines what a single Step is and how it behaves in isolation. This document defines how Steps are wired together without a mandatory central orchestrator.

A concrete library implementing them is a **separate project** and is out of scope here, but MUST satisfy the requirements defined in Sections 3–9.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** in this document are to be interpreted as described in BCP 14 (RFC 2119 and RFC 8174) when, and only when, they appear in all capitals, as shown here.

---

# 3. Session Script

## 3.1 Definition

A Session Script is the static, declarative definition of a coordinated flow. It is the single source of truth for how Steps compose.

A Session Script MUST include:

- A `ScriptId`.  
- A human-readable session name.  
- A `ScriptVersion`.  
- An ordered set of Steps and their allowed transitions.  
- For each Step, the global message identifiers it consumes and produces.  
- For each Step, the global data-element identifiers it requires as input and produces as output (see Section 6).  

A Session Script MUST be versioned and immutable once published.

A Session Script MUST NOT contain runtime session data.

## 3.2 Message Identifiers

For each Step, the Session Script MUST bind global message identifiers to the four coordination messages defined in SOPC-001 (`Activation`, `Rollback`, `Done`, `Failed`).

These identifiers are global to the flow so that a Step's outputs can be matched to the next Step's inputs without direct coupling between Participants.

## 3.3 Start Declaration

A Session Script MAY declare a **start signal** and its trigger binding, which governs how a Session for this script is initiated (Session Initiation, SOPC-001 §5.8).

Where declared, the start declaration MUST identify:

- The global message identifier of the start signal that activates an initiation poll.  
- The identity of the first Step activated when a Session is created.  

The trigger condition itself (user action, timer, or other environmental condition) is evaluated by the initiator and is not part of the immutable script beyond the declared start signal it responds to. A Session Script that declares no start signal relies on an out-of-band initiator to create Sessions.

---

# 4. Session Script Interface (Abstract)

The Session Script is exposed through an abstract interface. The interface is normative; the concrete implementation is a separate library project.

## 4.1 Requirements

An implementation of the Session Script interface MUST:

- Be immutable once loaded for a given `(ScriptId, ScriptVersion)`.  
- Be free of runtime session data (the interface exposes definition, not state).  
- Be safe for concurrent read access by multiple Participants.  
- Report structural and dataflow validity (Section 6) before it is considered publishable.  

## 4.2 Methods

The interface SHALL expose at least the following methods, whose semantics are normative (signatures below are illustrative, language-neutral):

- `Identity() -> (ScriptId, ScriptVersion, Name)` — returns the immutable identity of the script.  
- `Steps() -> [StepId]` — returns the ordered set of Steps.  
- `Transitions(StepId) -> [Transition]` — returns allowed successor transitions for a Step, including explicit failure and compensation transitions.  
- `MessageBindings(StepId) -> { Activation, Rollback, Done, Failed }` — returns the global message identifiers bound to a Step's four coordination messages.  
- `Inputs(StepId) -> [DataElementId]` — returns the global data-element identifiers the Step requires as input.  
- `Outputs(StepId) -> [DataElementId]` — returns the global data-element identifiers the Step produces.  
- `Classification(DataElementId) -> Classification` — returns the data classification for a data element.  
- `Validate() -> ValidationResult` — performs the structural and dataflow validation of Section 6.  

All methods MUST be deterministic and side-effect free.

---

# 5. Deployment Interface

Deployment of a Session Script is exposed through a separate deployment interface, distinct from the read-only Session Script interface of Section 4.

An implementation of the deployment interface MUST:

- `Distribute(Script, [Target])` — deliver a Session Script to every participating service at deployment time.  
- `Verify(Target) -> VerificationResult` — confirm that a target holds a `ScriptVersion` compatible with the Steps it implements, failing deployment on mismatch.  
- `ActiveVersions() -> [(ScriptId, ScriptVersion)]` — enumerate versions currently deployed, so in-flight Sessions can continue on their originating version.  

The deployment interface MUST NOT mutate a published Session Script; version changes are always expressed as a new `ScriptVersion`.

---

# 6. Versioning and Deployment Distribution

A Session Script SHALL be distributed to every service participating in the coordination at deployment time.

Each participating service MUST verify, at deployment, that it holds a Session Script version compatible with the Steps it implements. A version mismatch detected at deployment MUST fail the deployment or be surfaced as an explicit operational error.

Every coordination message MUST carry the `ScriptId` and `ScriptVersion` so mismatches are detectable at runtime, not only at deployment. A Participant that receives an `Activation` referencing a `ScriptVersion` it does not recognize MUST emit `Failed` (fail-fast) rather than execute against an assumed definition.

Breaking changes to a Session Script MUST be published as a new `ScriptVersion`. In-flight Sessions MUST continue to run against the `ScriptVersion` under which they were started.

---

# 7. Coordination and Transitions

Transitions are inferred from the Session Script and observed Session Log events; no mandatory central runtime drives them.

A transition to the next Step SHALL occur only after the required predecessor Step outcomes (`Done`) are present in the Session Log.

Failure transitions MUST be explicit in the Session Script.

Compensation transitions (`Rollback`, per SOPC-001 §5.4) SHOULD be defined for every Step that produces externally visible side effects, and MUST define the reverse order in which completed predecessor Steps are compensated.

## 7.1 Session Coordinator

The runtime role that observes the Session Log, consults the Session Script, and emits the next `Activation` (or `Rollback`) is the **Session Coordinator**.

The Session Coordinator MUST be **flow-agnostic**: it is a generic driver parameterized entirely by the Session Script. It MUST NOT embed flow-specific logic — no per-flow state enumeration and no per-flow branching. All flow knowledge lives in the Session Script (data); the Coordinator only reads the Script and the Session Log to decide what happens next.

The Session Coordinator MAY be realized in either of two forms, and this choice is an implementation concern:

- **Distributed** — each Participant reacts to predecessor `Done` events and self-activates its Steps, with no central component.  
- **Centralized** — a single generic driver reads the Session Log and emits the next activation.  

A centralized Session Coordinator is permitted precisely because it carries no flow-specific logic; it is therefore not a central orchestrator in the sense SOPC deprecates (SOPC-001 §3, SOPC-002 §11). A component that embeds flow-specific transitions is not a Session Coordinator but a reintroduction of the central-coordinator anti-pattern.

---

# 8. Global Data-Element Mapping

## 8.1 Purpose

The Session Script MUST declare, per Step, the global data-element identifiers the Step requires as input and the identifiers it produces as output. This turns the script into a dataflow contract in addition to a control-flow contract, enabling a bird's-eye view of what each Step consumes and produces.

## 8.2 Constraints

The mapping MUST reference data elements by global identifier and classification only. It MUST NOT define the internal value structure of a data element; that remains a Step concern per SOPC-001 §7.1. Encoding of the data element on the wire and at rest follows Section 9.

Publication of a Session Script MUST perform a static dataflow validation: every required input identifier for a Step MUST be produced by some predecessor Step in the flow. A Session Script that fails this validation MUST NOT be published.

Data classification attached to a global data-element identifier MAY drive redaction and least-privilege rules for derived views (SOPC-001 §10).

---

# 9. Global Data-Element Definitions

## 9.1 Globally Unique Identity

Every data element in the flow MUST be identified by a **globally unique identifier** (`DataElementId`). The identifier scheme MUST guarantee uniqueness without central coordination; a UUID is a suitable choice, but the specific scheme is left to the implementation.

A `DataElementId` is the single reference used everywhere the element appears: in the Session Script mapping (Section 8), in `Query` responses (Section 10), and in any serialized form.

## 9.2 Rules

A data element MAY be **primitive** (a single value) or **composite** (a set of nested, individually identified data elements).

A reader MUST ignore data elements whose identifier it does not recognize rather than fail, preserving forward compatibility (per SOPC-001 §6.2 schema evolution).

The identifier used in any serialized form MUST be the same `DataElementId` declared in the Session Script data-element mapping (Section 8), so that the data is self-describing against the script's bird's-eye view.

Any serialized representation is a wire and at-rest form only; the normative state contract remains the typed value objects of SOPC-001 §7.1. The concrete serialization is an implementation detail. A self-describing, length-delimited encoding keyed by the global identifier — for example, a BER-TLV-style tag/length/value structure in the spirit of EMV — is a reasonable direction, but no specific byte format is mandated by this RFC.

---

# 10. Query and Derivation

The `Query` message (SOPC-001 §5.3, §7.3) returns Step-local data by global data-element identifier. Because the Session Script declares which identifiers each Step produces, a caller can derive a complete Session data view by collecting the declared outputs across Steps.

Derived business artifacts (for example, receipt or ticket views) MAY be produced from projections over the Session Log, using the Session Script's data-element declarations as the schema of available fields.

---

# 11. Relationship to a Central Coordinator (Non-Normative)

This section is non-normative. It records the design intent that the Session Script **replaces a central coordinator** — a single, ever-growing state machine that owns every transition of a flow. Such a monolith is the tight-coupling anti-pattern SOPC-001 §3 sets out to remove.

## 11.1 The Mapping

A central coordinator typically concentrates several concerns in one place. Under SOPC these are separated:

| Central coordinator concern | SOPC replacement |
|---|---|
| One global state enumeration | Session Script Steps and transitions (§3, §7) |
| One dispatch switch owning all sequencing | Coordination inferred from Script + Session Log (§7); no central runtime |
| An idle/start state that begins new work | Initiator, a small runtime role (SOPC-001 §5.8) |
| A shared mutable state structure threaded through every branch | Session data, copied by value into declared data-elements (SOPC-001 §7.5, §8) |
| Sub-state-machines invoked by the switch | Steps / Participants (SOPC-001 §5.3, §5.6) |
| Long-lived device/health handling | Health Capability (SOPC-001 §5.7) |

## 11.2 Where Reuse Comes From

Existing sub-state-machines are usually already close to Steps: each has its own internal states and a terminal outcome that maps onto `Done` or `Failed`. What binds them into a monolith is not their logic but two couplings:

1. a shared mutable state structure, and  
2. a central switch that owns all sequencing.  

Removing both — shared state via copy-by-value data-elements, sequencing via the Session Script — allows each sub-state-machine to become an independently deployable, reusable Step. Device-access and health code is reused as-is, reframed as a Health Capability plus device access.

## 11.3 Incremental Migration

Deprecating a central coordinator need not be a single cutover. A flow MAY be migrated one Step at a time: extract a sub-state-machine behind the four coordination messages (`Activation`, `Rollback`, `Done`, `Failed`) and let the Session Script sequence it, while the legacy coordinator still drives the remaining, not-yet-extracted portions. As Steps are extracted, the central state enumeration shrinks toward the initiator's idle role and is finally retired.

---

# 12. Open Questions

Genuine spec-level questions:

- Should a Start Declaration (§3.3) be mandatory for self-contained scripts, or remain optional to allow out-of-band initiators?  
- Should the Session Script mandate parallel execution of independent (non-dependent) Steps, or leave concurrency to the implementation?  

Deferred to the Session Script "manager" implementation, not this RFC:

- Session Script serialization format (JSON Schema, YAML) and whether it is exposed to callers.  
- The transport used by the deployment interface for `Distribute`.  
- Ordered delivery guarantees per Session partition key.  
- Projection storage pattern for derived views (a general persistence concern, e.g. receipt and ticket generation).  

---

# 13. References

[1] S. Bradner, "Key words for use in RFCs to Indicate Requirement Levels," BCP 14, RFC 2119, March 1997.

[2] B. Leiba, "Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words," BCP 14, RFC 8174, May 2017.

[3] H. Garcia-Molina and K. Salem, "Sagas," *ACM SIGMOD Record*, vol. 16, no. 3, pp. 249-259, 1987. https://doi.org/10.1145/38713.38742

[4] SOPC-001, "Session-Oriented Process Coordination — Core Model."
