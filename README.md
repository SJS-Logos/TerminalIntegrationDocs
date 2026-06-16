# Terminal Integration Docs

Technical specifications for integration between an embedded teminal and backend

---

## RFC Index

| Title | Version | Status |
|-------|---------|--------|
| [Http Request Execution](docs/HttpRequestExecution.md) | 1.0 | Draft |
| [Notification Bridge For LongPolling And MQTT] (docs/NotificationBridgeForLongPollingAndMQTT.md) | 1.0 | Draft |

---

## Abstracts

### HTTP Request Execution, Idempotency, and Operation Lifecycle Protocol

This specification defines a transport-layer protocol for reliable execution of client-initiated operations over HTTP in environments with unreliable connectivity (e.g., embedded terminals).

It introduces three identifiers:

* Idempotency Key (request deduplication)
* Operation ID (server-side execution tracking)
* Correlation ID (distributed tracing)

It also defines an Operation resource model and strict rules for 202 Accepted semantics to ensure crash safety and recoverability.

→ [Read full RFC](docs/HttpRequestExecution.md)

### Minimal Notification Bridge for LongPolling and MQTT

This specification defines a minimal message format and transport bridge for delivering notifications to terminals via LongPolling and MQTT.

The system is intentionally designed as a **signal-only mechanism**, where notifications do not carry domain state, but instead act as triggers for state retrieval via REST APIs.

→ [Read full RFC](docs/NotificationBridgeForLongPollingAndMQTT.md)
