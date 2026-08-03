# RFC: Session-Oriented Process Coordination (SOPC)

**Version:** 0.1  
**Status:** Draft  
**Author:** SJS  
**Created:** 2026-08-03  

---

# 1. Introduction

This RFC defines a shared abstraction for coordinating long-running, stateful interactions across UI, device, and distributed service boundaries.

The model generalizes Saga-style behavior without introducing a mandatory central orchestrator. Coordination is driven by append-only session events and local participant logic.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** in this document are to be interpreted as described in BCP 14 (RFC 2119 and RFC 8174) when, and only when, they appear in all capitals, as shown here.

---

# 3. Motivation

Distributed flows often become tightly coupled when one central runtime controls all step transitions.

SOPC aims to preserve local autonomy while keeping end-to-end flow semantics explicit, auditable, and recoverable.

---

# 4. Scope

In scope:

- A static flow definition model.  
- A runtime session model for stateful execution.  
- Step activation, completion, failure, and compensation signaling.  
- An append-only, versioned session log as the shared coordination surface.  
- Query patterns for deriving audit artifacts such as receipts and tickets.  

Out of scope:

- Global ACID guarantees across independent services.  
- A mandated broker or log implementation technology.  
- UI rendering details or transport-specific API contracts.  

---

# 5. Core Concepts

## 5.1 Flow Definition (Static)

A Flow Definition declares ordered steps and allowed transitions.

A Flow Definition MUST be versioned and immutable once published.

A Flow Definition MUST NOT contain runtime session data.

## 5.2 Session (Runtime)

A Session represents one execution instance of a Flow Definition.

A Session SHALL include at least:

- `SessionId`  
- `FlowId`  
- `FlowVersion`  
- `CurrentStep`  
- `State`  
- `HistoryPointer`  

A Session MUST be reconstructable from its Session Log.

## 5.3 Step

A Step is an atomic unit of interaction.

A Step lifecycle consists of:

1. Activation  
2. Execution  
3. Terminal outcome (`Done` or `Failed`)  
4. Optional compensation (`Rollback`)  

A Step implementation MUST be idempotent.

## 5.4 Participant

A Participant executes one or more Step handlers.

A Participant SHOULD subscribe to activation and rollback signals relevant to its steps.

A Participant SHALL publish terminal outcome events (`Done` or `Failed`) for each activated step.

---

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

# 7. State Model

Session state MUST be represented as typed, immutable value objects.

A free-form dictionary MAY be used only as an internal adapter detail and MUST NOT be the normative Session state contract.

Each Step MAY contribute additional value objects to Session state, but MUST NOT mutate prior events.

---

# 8. Coordination and Transitions

Transitions are inferred from the Flow Definition and observed Session Log events.

A transition to the next Step SHALL occur only after required predecessor Step outcomes are present.

Failure transitions MUST be explicit in the Flow Definition.

Compensation transitions SHOULD be defined for every Step that produces externally visible side effects.

---

# 9. Query and Derivation

The platform MUST support querying by `SessionId`.

The platform MUST support querying by Step within a Session.

The platform SHOULD support filtering by event type and time range.

Derived business artifacts (for example, receipt or ticket views) MAY be produced from projections over the Session Log.

---

# 10. Reliability and Failure Handling

Delivery semantics MAY be at-least-once.

Consumers MUST deduplicate by event identifier.

Retries SHOULD use bounded backoff policies.

If a Step cannot complete after retries, the Participant MUST emit a `Failed` event and include failure metadata sufficient for compensation or operator handling.

---

# 11. Security and Compliance

Sensitive payload fields MUST be classified and protected at rest and in transit.

Access to Session and Session Log data MUST be scoped by least privilege.

Redaction rules SHOULD be defined for derived views used outside trusted operational boundaries.

---

# 12. Relationship to Sagas

A Saga is a specialization of SOPC where the flow concerns distributed business transactions.

SOPC generalizes this model to any stateful interaction, including UI wizards, terminal flows, provisioning workflows, and device update pipelines.

---

# 13. Open Questions

- Should Flow Definition serialization (JSON Schema, YAML) be exposed to callers  or kept internal?  
- Should ordered delivery be required per Session partition key, or only recommended?  
- Which projection storage pattern should be the default for receipt and ticket generation?  

---

# 14. References

[1] S. Bradner, "Key words for use in RFCs to Indicate Requirement Levels," BCP 14, RFC 2119, March 1997.

[2] B. Leiba, "Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words," BCP 14, RFC 8174, May 2017.

[3] H. Garcia-Molina and K. Salem, "Sagas," *ACM SIGMOD Record*, vol. 16, no. 3, pp. 249-259, 1987. https://doi.org/10.1145/38713.38742
