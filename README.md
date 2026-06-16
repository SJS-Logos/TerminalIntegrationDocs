# Terminal Integration Docs

Technical specifications for integration between an embedded teminal and backend

---

## RFC Index

| Title | Version | Status |
|-------|---------|--------|
| [Http Request Execution](Concepts/HttpRequestExecution/RFC.md) | 1.0 | Draft |

---

## Abstracts

### HTTP Request Execution, Idempotency, and Operation Lifecycle Protocol

This specification defines a transport-layer protocol for reliable execution of client-initiated operations over HTTP in environments with unreliable connectivity (e.g., embedded terminals).

It introduces three identifiers:

* Idempotency Key (request deduplication)
* Operation ID (server-side execution tracking)
* Correlation ID (distributed tracing)

It also defines an Operation resource model and strict rules for 202 Accepted semantics to ensure crash safety and recoverability.

→ [Read full RFC](Concepts/HttpRequestExecution/RFC.md)

