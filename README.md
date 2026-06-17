# Terminal Integration Docs

Technical specifications for integration between an embedded teminal and backend

---

## RFC Index

| Title | Version | Status |
|-------|---------|--------|
| [Idempotent Command Execution Protocol](docs/IdempotentCommandExecutionProtocol.md) | 1.0 | Draft |
| [Notification Bridge For LongPolling And MQTT](docs/NotificationBridgeForLongPollingAndMQTT.md) | 1.0 | Draft |

---

## Abstracts

### Idempotent Command Execution Protocol

This specification defines an HTTP-based execution protocol for reliable command processing in distributed systems with unreliable clients and intermittent connectivity.

The protocol provides:

- request deduplication  
- durable server-side execution tracking  
- asynchronous execution support  
- recovery from client and server failures  

This specification explicitly defines a model that does not require a dedicated Operations endpoint.

→ [Read full RFC](docs/IdempotentCommandExecutionProtocol.md)

### Minimal Notification Bridge for LongPolling and MQTT

This specification defines a minimal message format and transport bridge for delivering notifications to terminals via LongPolling and MQTT.

The system is intentionally designed as a **signal-only mechanism**, where notifications do not carry domain state, but instead act as triggers for state retrieval via REST APIs.

→ [Read full RFC](docs/NotificationBridgeForLongPollingAndMQTT.md)
