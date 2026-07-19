# RFC: Minimal Notification Bridge for LongPolling and MQTT

**Version:** 0.1  
**Status:** Draft — Feedback / Counter-proposal  

> **How this feedback is being raised:** The RFC this document responds to is owned by the `AiO-IntegrationDocs` repository, where the author does not have branch/PR rights. This feedback is therefore hosted in `TerminalIntegrationDocs` and raised with the AiO architecture team **by reference** (a link to this document) rather than as a pull request against their repository. This note records that provenance for future readers.

---

# 0. Relationship to "LongPolling Downstream Transport for the Notification Orchestration Service"

This document is **feedback on**, and a **scope counter-proposal to**, the RFC *"LongPolling Downstream Transport for the Notification Orchestration Service"* (NOS LongPolling Transport RFC, v1.0, Draft).

It is not an independent, competing service design. Its purpose is to challenge a single foundational decision in that RFC — **what the downstream transport actually delivers** — and to show how narrowing that scope collapses much of the mechanism the target RFC treats as essential.

## 0.1 Points of Agreement

This feedback is a **scope narrowing**, not a rejection of the target RFC's engineering. The following decisions are sound and are **not** disputed:

- HTTP long polling over mTLS as a downstream transport for terminals restricted to outbound HTTP (target §3).
- Redis-backed queuing with pod-targeted wakeup instead of Redis Pub/Sub broadcast (target §10.2).
- The "infrastructure before domain" ordering to close the wakeup race condition (target §10.3).
- The pod-failure and wakeup-failure recovery flows (target §11).

The disagreement is confined to **what the transport carries**, addressed next.

## 0.2 The Core Scope Disagreement

The target RFC scopes the transport as a **reliable carrier of notification content**. Its envelope (§6.2) embeds the *full serialized payload* (e.g. `saleId`, `amount`, `currency`), and the surrounding machinery — at-least-once QoS, ACK tracking via `lastAckedId` (§6.3), 30-day retention (§6.1), and the accepted data-loss risk analysis (§12) — exists to deliver that payload reliably.

This document argues the downstream transport should instead be scoped as a **signal-only invalidation channel**: notifications carry only *what changed* (`keyword` + `objectId` + `applicationId`), and the terminal retrieves authoritative state via REST (§4, §10). The payload is deliberately excluded.

## 0.3 Why the Scope Matters

If notifications are signal-only, most of the target RFC's essential complexity becomes unnecessary:

| Target RFC mechanism | Justified by payload delivery | Needed for signal-only? |
| --- | --- | --- |
| At-least-once QoS + ACK protocol (§6.3) | Guarantee payload is not lost | No — idempotent REST retrieval makes at-most-once sufficient |
| `lastAckedId` queue trimming (§6.3) | Track payload delivery | No — no per-message state to track |
| 30-day notification TTL (§6.1, §13) | Retain undelivered payloads | No — a stale "something changed" signal has no value |
| Accepted data-loss risk (§12) | Payload can be lost on crash | Not applicable — signals are safely reconstructable via REST |

The disagreement is therefore not about Redis, pod-targeted wakeup, or long polling mechanics — those are reasonable — but about **whether the transport carries state at all**. That single decision determines whether the QoS/ACK/retention apparatus is required or redundant.

## 0.4 Additional Scope Observations

- **Transport lock-in (target §1, §4).** By scoping the RFC to LongPolling specifically, the notification contract is coupled to one transport. This document treats the signal format as transport-independent (§4.3), with LongPolling (PCI-approved) and MQTT (non-PCI) as interchangeable adapters over the same AMQP-originated signal.
- **"Opaque payload" is a symptom (target §4.2, §6.2).** The target RFC treats the payload as opaque yet still transports, persists, and retries it for 30 days. If it is genuinely opaque to the transport, it does not belong in the transport — reinforcing the signal-only position.

## 0.5 Envelope Vocabulary Mapping

The two documents use different field names for the notification envelope. The mapping below aligns the target RFC's envelope (§6.2) with the signal-only envelope proposed here (§6):

| Target RFC field (§6.2) | This document | Disposition |
| --- | --- | --- |
| `type` | `keyword` | Renamed; same role (identifies the kind of change) |
| `id` | (none) | Dropped; needed only for ACK tracking, which signal-only removes |
| `timestamp` | (none) | Dropped; not required for an invalidation signal |
| `payload` | (none) | **Removed** — this is the core change; state is retrieved via REST |
| (none) | `objectId` | **Added** — stable identifier the terminal uses for REST retrieval |
| (none) | `applicationId` | **Added** — terminal/device identity for routing (implicit in the target's `deviceAppId` path segment) |

The net effect is that the envelope loses `payload`/`id`/`timestamp` and gains `objectId`, turning a data carrier into a routing-and-retrieval hint.

The remainder of this document specifies the signal-only model in full as the proposed alternative scope.

---

# 1. Introduction

This specification defines a minimal message format and transport bridge for delivering notifications to terminals via LongPolling and MQTT.

The system is intentionally designed as a **signal-only mechanism**, where notifications do not carry domain state, but instead act as triggers for state retrieval via REST APIs.

## 1.1 Background and Motivation

AMQP has taken on a larger role as the coordination fabric between backend services. To let embedded terminal services participate in that same coordination, this bridge translates AMQP notifications into transport mechanisms that embedded terminals can consume.

Two transports are supported because they serve different deployment contexts:

- **LongPolling** uses HTTP, which is an approved protocol inside the PCI-regulated environment.
- **MQTT** offers efficient publish/subscribe delivery for terminals operating outside PCI scope.

MQTT is not permitted inside the PCI-regulated environment: PCI allows only an approved whitelist of protocols, and MQTT is not on that list. This is an allowed-protocol (compliance) constraint, not a security judgement about MQTT. LongPolling therefore exists as the mandatory transport for terminals operating within PCI scope. See [Transport Applicability](#5-transport-applicability).

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**, **SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **MAY**, and **OPTIONAL** in this document are to be interpreted as described in RFC 2119 and RFC 8174, and only when they appear in all capitals.

---

# 3. Scope

## 3.1 In Scope

- Minimal AMQP message format for notifications  
- Subscription model for LongPolling and MQTT controllers  
- Translation of AMQP messages into transport-specific messages  
- Terminal notification delivery semantics  
- REST API-based state retrieval pattern  

## 3.2 Out of Scope

- Domain event schemas  
- Business payload content  
- Message routing logic beyond keyword matching  
- LongPolling persistence internals  
- MQTT broker configuration  

---

# 4. Design Principles

## 4.1 Signal-Only Messaging

AMQP messages defined in this specification MUST NOT contain business state.

They function solely as:

> A signal that state has changed and MUST be retrieved via REST API.

---

## 4.2 REST as Source of Truth

The terminal MUST use REST APIs to retrieve the actual state associated with a notification.

Notifications are NOT authoritative data carriers.

> **Reference:** Using REST as the authoritative, queryable source of truth follows the REST architectural style ([Fielding][fielding-rest]) and the Event Notification pattern ([Fowler][fowler-event-notification]), in which an event signals only that state changed while the consumer retrieves current state via a subsequent request (e.g. HTTP GET, [RFC 9110 §9.3.1][rfc9110-get]).

---

## 4.3 Transport Independence

The AMQP message format is independent of:

- LongPolling implementation  
- MQTT implementation  
- downstream delivery mechanism  

---

# 5. Transport Applicability

## 5.1 PCI Protocol Whitelist

PCI permits only an approved whitelist of protocols. MQTT is not on that whitelist.

- Terminals operating **within** the PCI-regulated environment **MUST** use LongPolling (HTTP), which is an approved protocol.
- Such terminals **MUST NOT** use MQTT.
- MQTT **MAY** be used only for terminals operating **outside** PCI scope.

This restriction is a compliance (allowed-protocol) requirement and is independent of any security property of MQTT.

> **Note:** The authoritative set of approved protocols is defined and maintained by the organization's PCI compliance authority (the PCI regulatory team), not by this specification. Refer to the current internal PCI approved-protocols standard for the definitive list. A specific external citation is intentionally omitted here pending confirmation of the exact source from the PCI regulatory team.

<!-- TODO(PCI): Replace the note above with a citation to the authoritative PCI approved-protocols standard once the PCI regulatory team provides the exact reference. -->

## 5.2 Transport Equivalence

Both transports deliver the identical signal-only message defined in this specification. A terminal's choice of transport **MUST NOT** change notification semantics or the requirement to retrieve authoritative state via REST.

---

# 6. AMQP Message Format

## 6.1 Minimal Notification Message

All notifications MUST conform to the following structure:

```json
{
  "keyword": "<string>",
  "applicationId": "<string>",
  "objectId": "<string>"
}
```

---

## 6.2 Field Definitions

### keyword

Represents the type of state change.

Examples:

- `"FinancialLogCreated"`  
- `"ReservationUpdated"`  
- `"DeviceStatusChanged"`  

The keyword MUST be treated as a routing hint only.

---

### applicationId

Identifies the terminal or client application.

This corresponds to the terminal identity (e.g. DeviceId / ApplicationId).

---

### objectId

Identifies the affected resource.

This is a stable identifier used for REST API retrieval.

---

## 6.3 Constraints

- Messages MUST NOT contain state payloads  
- Messages MUST NOT contain computed results  
- Messages MUST NOT embed REST responses  
- Messages MUST remain minimal and opaque to transport layer  

---

# 7. AMQP Subscription Model

## 7.1 Notification Controller Subscription

A Notification Controller (LongPolling or MQTT bridge) MUST:

- subscribe to AMQP messages matching allowed keywords  
- filter messages by `applicationId`  
- forward matching messages to the appropriate transport adapter  

---

## 7.2 Routing Rule

A message MUST be forwarded if:

- `applicationId` matches an active subscriber session  
- keyword is registered for that controller  

---

# 8. LongPolling Controller Behavior

## 8.1 Message Transformation

When a matching AMQP message is received, the LongPolling Controller MUST:

1. Convert AMQP message into a LongPolling notification envelope  
2. Store or buffer the notification for the associated terminal session  
3. Trigger wake-up delivery via LongPolling API  

---

## 8.2 Notification Envelope

The LongPolling message MUST include:

- keyword  
- objectId  
- applicationId  

No additional state is allowed.

---

## 8.3 Delivery Semantics

The controller MUST NOT assume:

- terminal is actively polling  
- message is immediately consumed  

If no active poll exists:

> the message MAY be buffered until next poll cycle

> **Reference:** The LongPolling delivery model used here as a change indicator follows the long-polling best practices described in [RFC 6202][rfc6202].

---

# 9. MQTT Controller Behavior

## 9.1 Transformation Rule

The MQTT Controller MUST:

- subscribe to the same AMQP message stream  
- transform messages into MQTT publish events  
- publish to topic derived from `applicationId`  

> **Reference:** The MQTT delivery model used here as a change indicator follows the publish/subscribe semantics of [MQTT Version 5.0][mqtt-v5], an instance of the Publish-Subscribe Channel pattern ([Enterprise Integration Patterns][eip-pubsub]).

---

## 9.2 MQTT Topic Convention

MQTT topics SHOULD follow:

```text
/notification/{applicationId}
```

---

## 9.3 Payload Constraints

MQTT payload MUST be identical to AMQP message structure:

```json
{
  "keyword": "...",
  "applicationId": "...",
  "objectId": "..."
}
```

---

# 10. Terminal Behavior

## 10.1 Primary Rule

The terminal MUST treat all notifications as:

> "State change indicator — retrieve updated state via REST API"

---

## 10.2 Required Action Pattern

Upon receiving a notification:

1. Parse `keyword`  
2. Identify affected resource via `objectId`  
3. Call corresponding REST endpoint  
4. Discard notification after processing  

---

## 10.3 Idempotency

The terminal MUST assume:

- notifications MAY be duplicated  
- notifications MAY arrive out of order  
- notifications MAY be replayed  

Therefore:

> REST API response is the only authoritative state

---

# 11. Failure Handling

## 11.1 Lost Notifications

If AMQP message is not delivered:

- no system state is corrupted  
- terminal will eventually reconcile via REST polling or next notification  

---

## 11.2 Duplicate Notifications

Duplicates MUST be allowed.

Terminal MUST ensure idempotent handling via REST retrieval.

---

## 11.3 Controller Failure

If LongPolling or MQTT controller fails:

- AMQP messages remain in broker  
- messages MAY be retried or reprocessed  
- no message carries critical state  

---

# 12. Rationale

## 12.1 Why minimal AMQP format?

To ensure:

- transport independence  
- low coupling to domain evolution  
- predictable routing behavior  

---

## 12.2 Why no payload?

Because payload inclusion would:

- duplicate REST responsibilities  
- create inconsistent state views  
- couple transport to domain schema evolution  

---

## 12.3 Why REST is required?

Because:

> REST provides authoritative, queryable, consistent state

AMQP only signals that a change occurred.

---

## 12.4 Why keyword-based routing?

Because:

- it avoids schema-aware brokers  
- keeps controllers stateless  
- enables simple filtering logic  

---

# 13. Design Summary

| Component              | Responsibility              |
| ---------------------- | --------------------------- |
| AMQP message           | minimal state change signal |
| LongPolling controller | delivery adapter            |
| MQTT controller        | publish adapter             |
| REST API               | source of truth             |
| Terminal               | state reconciliation client |

---

# 14. Final Principle

> Notifications are not data. Notifications are invalidation signals.

---

# 15. References

- [Roy T. Fielding — Architectural Styles and the Design of Network-based Software Architectures (REST), Ch. 5][fielding-rest]  
- [RFC 9110 — HTTP Semantics, §9.3.1 GET][rfc9110-get]  
- [Martin Fowler — What do you mean by Event-Driven? (Event Notification)][fowler-event-notification]  
- [OASIS — MQTT Version 5.0][mqtt-v5]  
- [RFC 6202 — Best Practices for Long Polling and Streaming in Bidirectional HTTP][rfc6202]  
- [OASIS — AMQP Version 1.0][amqp-v1]  
- [Enterprise Integration Patterns — Publish-Subscribe Channel][eip-pubsub]  
- [RFC 2119 — Key words for use in RFCs to Indicate Requirement Levels][rfc2119]  
- [RFC 8174 — Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words][rfc8174]  

[fielding-rest]: https://www.ics.uci.edu/~fielding/pubs/dissertation/rest_arch_style.htm
[rfc9110-get]: https://www.rfc-editor.org/rfc/rfc9110#section-9.3.1
[fowler-event-notification]: https://martinfowler.com/articles/201701-event-driven.html
[mqtt-v5]: https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html
[rfc6202]: https://www.rfc-editor.org/rfc/rfc6202
[amqp-v1]: https://docs.oasis-open.org/amqp/core/v1.0/amqp-core-messaging-v1.0.html
[eip-pubsub]: https://www.enterpriseintegrationpatterns.com/patterns/messaging/PublishSubscribeChannel.html
[rfc2119]: https://www.rfc-editor.org/rfc/rfc2119
[rfc8174]: https://www.rfc-editor.org/rfc/rfc8174