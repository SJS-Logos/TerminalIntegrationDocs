# RFC: Minimal Notification Bridge for LongPolling and MQTT

**Version:** 1.0  
**Status:** Draft  

---

# 1. Introduction

This specification defines a minimal message format and transport bridge for delivering notifications to terminals via LongPolling and MQTT.

The system is intentionally designed as a **signal-only mechanism**, where notifications do not carry domain state, but instead act as triggers for state retrieval via REST APIs.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** are to be interpreted as described in RFC 2119.

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

# 5. AMQP Message Format

## 5.1 Minimal Notification Message

All notifications MUST conform to the following structure:

```json
{
  "keyword": "<string>",
  "applicationId": "<string>",
  "objectId": "<string>"
}
```

---

## 5.2 Field Definitions

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

## 5.3 Constraints

- Messages MUST NOT contain state payloads  
- Messages MUST NOT contain computed results  
- Messages MUST NOT embed REST responses  
- Messages MUST remain minimal and opaque to transport layer  

---

# 6. AMQP Subscription Model

## 6.1 Notification Controller Subscription

A Notification Controller (LongPolling or MQTT bridge) MUST:

- subscribe to AMQP messages matching allowed keywords  
- filter messages by `applicationId`  
- forward matching messages to the appropriate transport adapter  

---

## 6.2 Routing Rule

A message MUST be forwarded if:

- `applicationId` matches an active subscriber session  
- keyword is registered for that controller  

---

# 7. LongPolling Controller Behavior

## 7.1 Message Transformation

When a matching AMQP message is received, the LongPolling Controller MUST:

1. Convert AMQP message into a LongPolling notification envelope  
2. Store or buffer the notification for the associated terminal session  
3. Trigger wake-up delivery via LongPolling API  

---

## 7.2 Notification Envelope

The LongPolling message MUST include:

- keyword  
- objectId  
- applicationId  

No additional state is allowed.

---

## 7.3 Delivery Semantics

The controller MUST NOT assume:

- terminal is actively polling  
- message is immediately consumed  

If no active poll exists:

> the message MAY be buffered until next poll cycle

> **Reference:** The LongPolling delivery model used here as a change indicator follows the long-polling best practices described in [RFC 6202][rfc6202].

---

# 8. MQTT Controller Behavior

## 8.1 Transformation Rule

The MQTT Controller MUST:

- subscribe to the same AMQP message stream  
- transform messages into MQTT publish events  
- publish to topic derived from `applicationId`  

> **Reference:** The MQTT delivery model used here as a change indicator follows the publish/subscribe semantics of [MQTT Version 5.0][mqtt-v5], an instance of the Publish-Subscribe Channel pattern ([Enterprise Integration Patterns][eip-pubsub]).

---

## 8.2 MQTT Topic Convention

MQTT topics SHOULD follow:

```text
/notification/{applicationId}
```

---

## 8.3 Payload Constraints

MQTT payload MUST be identical to AMQP message structure:

```json
{
  "keyword": "...",
  "applicationId": "...",
  "objectId": "..."
}
```

---

# 9. Terminal Behavior

## 9.1 Primary Rule

The terminal MUST treat all notifications as:

> "State change indicator — retrieve updated state via REST API"

---

## 9.2 Required Action Pattern

Upon receiving a notification:

1. Parse `keyword`  
2. Identify affected resource via `objectId`  
3. Call corresponding REST endpoint  
4. Discard notification after processing  

---

## 9.3 Idempotency

The terminal MUST assume:

- notifications MAY be duplicated  
- notifications MAY arrive out of order  
- notifications MAY be replayed  

Therefore:

> REST API response is the only authoritative state

---

# 10. Failure Handling

## 10.1 Lost Notifications

If AMQP message is not delivered:

- no system state is corrupted  
- terminal will eventually reconcile via REST polling or next notification  

---

## 10.2 Duplicate Notifications

Duplicates MUST be allowed.

Terminal MUST ensure idempotent handling via REST retrieval.

---

## 10.3 Controller Failure

If LongPolling or MQTT controller fails:

- AMQP messages remain in broker  
- messages MAY be retried or reprocessed  
- no message carries critical state  

---

# 11. Rationale

## 11.1 Why minimal AMQP format?

To ensure:

- transport independence  
- low coupling to domain evolution  
- predictable routing behavior  

---

## 11.2 Why no payload?

Because payload inclusion would:

- duplicate REST responsibilities  
- create inconsistent state views  
- couple transport to domain schema evolution  

---

## 11.3 Why REST is required?

Because:

> REST provides authoritative, queryable, consistent state

AMQP only signals that a change occurred.

---

## 11.4 Why keyword-based routing?

Because:

- it avoids schema-aware brokers  
- keeps controllers stateless  
- enables simple filtering logic  

---

# 12. Design Summary

| Component              | Responsibility              |
| ---------------------- | --------------------------- |
| AMQP message           | minimal state change signal |
| LongPolling controller | delivery adapter            |
| MQTT controller        | publish adapter             |
| REST API               | source of truth             |
| Terminal               | state reconciliation client |

---

# 13. Final Principle

> Notifications are not data. Notifications are invalidation signals.

---

# 14. References

- [Roy T. Fielding — Architectural Styles and the Design of Network-based Software Architectures (REST), Ch. 5][fielding-rest]  
- [RFC 9110 — HTTP Semantics, §9.3.1 GET][rfc9110-get]  
- [Martin Fowler — What do you mean by Event-Driven? (Event Notification)][fowler-event-notification]  
- [OASIS — MQTT Version 5.0][mqtt-v5]  
- [RFC 6202 — Best Practices for Long Polling and Streaming in Bidirectional HTTP][rfc6202]  
- [OASIS — AMQP Version 1.0][amqp-v1]  
- [Enterprise Integration Patterns — Publish-Subscribe Channel][eip-pubsub]  
- [RFC 2119 — Key words for use in RFCs to Indicate Requirement Levels][rfc2119]  

[fielding-rest]: https://www.ics.uci.edu/~fielding/pubs/dissertation/rest_arch_style.htm
[rfc9110-get]: https://www.rfc-editor.org/rfc/rfc9110#section-9.3.1
[fowler-event-notification]: https://martinfowler.com/articles/201701-event-driven.html
[mqtt-v5]: https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html
[rfc6202]: https://www.rfc-editor.org/rfc/rfc6202
[amqp-v1]: https://docs.oasis-open.org/amqp/core/v1.0/amqp-core-complete-v1.0.html
[eip-pubsub]: https://www.enterpriseintegrationpatterns.com/patterns/messaging/PublishSubscribeChannel.html
[rfc2119]: https://www.rfc-editor.org/rfc/rfc2119
