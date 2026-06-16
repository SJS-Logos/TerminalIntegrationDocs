# RFC: Idempotent Command Execution Protocol (No Operation Resource Model)

**Version:** 1.0
**Status:** Draft

---

# 1. Introduction

This specification defines an HTTP-based execution protocol for reliable command processing in distributed systems with unreliable clients and intermittent connectivity.

The protocol provides:

* request deduplication
* durable server-side execution tracking
* asynchronous execution support
* recovery from client and server failures

This specification explicitly defines a model that does not require a dedicated Operations endpoint.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **SHALL**, **SHOULD**, and **MAY** are to be interpreted as described in RFC 2119.

---

# 3. Scope

## 3.1 In Scope

* HTTP request execution semantics
* Idempotency-based request deduplication
* Asynchronous execution via `202 Accepted`
* Server-side execution state persistence requirements
* Retry and recovery semantics

## 3.2 Out of Scope

* Domain-specific models
* Business transaction semantics
* Storage implementation details
* Database schema design
* Dedicated operation query endpoints

---

# 4. Core Concepts

## 4.1 Execution Record

An Execution Record is a server-side durable representation of a command execution instance.

It contains:

* Idempotency-Key
* Request hash
* Execution state
* Response data (final or partial)
* Timestamps

---

## 4.2 Execution State

An Execution Record SHALL transition through the following states:

* Pending
* Running
* Completed
* Failed

State transitions MUST be monotonic and persisted durably.

---

## 4.3 Idempotency-Key

The Idempotency-Key is a client-generated identifier that uniquely represents a logical command execution.

It is used to ensure that repeated requests do not result in multiple executions.

---

## 4.4 Correlation-Id

The Correlation-Id is a trace identifier used for observability.

It MUST be propagated across service boundaries and MUST NOT influence execution semantics.

---

# 5. HTTP Headers

## 5.1 Idempotency-Key

The client SHALL include an Idempotency-Key in all non-idempotent requests (e.g. POST, PATCH, DELETE).

### Requirements

* MUST be unique per logical command execution
* MUST be reused for retries of the same command
* MUST NOT be reused for different commands

---

## 5.2 Correlation-Id

The client MAY include a Correlation-Id.

If present:

* MUST be propagated unchanged by all services
* MUST NOT influence execution outcome

---

# 6. Execution Model

## 6.1 Synchronous Execution

If a command completes within the server’s response budget:

```http id="sync1"
200 OK
or
201 Created
```

No Execution Record retrieval is required by the client.

---

## 6.2 Asynchronous Execution

If a command cannot be completed within the response budget:

```http id="async1"
202 Accepted
```

### Requirements

The server:

* MUST persist the Execution Record BEFORE returning the response
* MUST associate the Idempotency-Key with the Execution Record
* MUST ensure the Execution Record is recoverable after failure

---

# 7. Idempotency Semantics

## 7.1 Duplicate Request Handling

If a request is received with an existing Idempotency-Key:

* The server MUST NOT re-execute the command
* The server MUST return the current Execution Record state
* The server MUST ensure response consistency across retries

---

## 7.2 Request Payload Consistency

If a request is received with an existing Idempotency-Key but a different payload:

* The server MUST reject the request
* The server MUST NOT modify existing Execution Records

---

# 8. Recovery Model

## 8.1 Client Failure Recovery

If a client loses the response to a request:

* The client SHALL retry the original request with the same Idempotency-Key
* The server SHALL return the existing Execution Record state

---

## 8.2 Server Failure Recovery

If the server crashes after issuing a 202 response:

* The Execution Record MUST be recoverable from durable storage
* The execution MUST resume or remain queryable via retrying the same request

---

## 8.3 Network Uncertainty

The client MUST treat the following as ambiguous outcomes:

* timeout
* connection loss
* missing response

In all cases, the client SHALL retry using the same Idempotency-Key.

---

# 9. Polling and Status Retrieval

This specification does not define a dedicated status endpoint.

Instead, clients SHALL use the idempotent POST request itself as the mechanism for status retrieval.

### Behavior

A repeated request with the same Idempotency-Key:

* MUST NOT trigger re-execution
* MUST return the current Execution Record state or final response

---

# 10. Role of Persistent Storage

## 10.1 Durable Storage Requirement

The server MUST persist Execution Records in durable storage before acknowledging a request with `202 Accepted`.

Failure to persist MUST result in request failure (no acknowledgment).

---

## 10.2 Ephemeral Storage (Optional)

Ephemeral storage (e.g. caches) MAY be used for optimization but:

* MUST NOT be the source of truth for execution state
* MUST NOT determine idempotency correctness
* MUST NOT be required for recovery

---

# 11. Failure Semantics

## 11.1 Crash Before Persistence

* No 202 MAY be returned
* Client retries safely

---

## 11.2 Crash After Persistence

* Execution Record MUST remain available
* Client retries return same state

---

## 11.3 Cache Loss

Cache loss MUST NOT affect correctness, only performance.

---

# 12. Design Principles

## 12.1 Execution Uniqueness

> Each Idempotency-Key corresponds to exactly one Execution Record.

---

## 12.2 Execution Persistence First

> A server MUST NOT acknowledge acceptance (202) without durable persistence of execution state.

---

## 12.3 Stateless Client Assumption

Clients MUST NOT rely on server-side session continuity.

All recovery is driven by Idempotency-Key replay.

---

## 12.4 No Operation Resource Requirement

This protocol explicitly does NOT require:

* Operation endpoints
* Operation identifiers exposed to clients
* Separate polling APIs

All state retrieval is performed through idempotent request replay.

---

# 13. Security Considerations

* Idempotency-Key reuse across different payloads MUST be rejected
* Execution records MUST be protected against unauthorized replay
* Correlation-Id MUST NOT be used for authorization decisions

---

# 14. Summary of Semantics

| Concept               | Mechanism              | Responsibility |
| --------------------- | ---------------------- | -------------- |
| Duplicate suppression | Idempotency-Key        | Server         |
| Execution tracking    | Execution Record       | Server         |
| Retry recovery        | Idempotent POST replay | Client         |
| Observability         | Correlation-Id         | Infrastructure |

---

# 15. Final Principle

> In this model, the idempotent request itself is the handle to both command execution and execution state retrieval.

---
