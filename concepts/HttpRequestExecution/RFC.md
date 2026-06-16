# RFC: HTTP Execution Metadata and Asynchronous Operation Protocol

## 1. Scope 

This specification defines a transport-level protocol for reliable execution of HTTP requests in distributed systems with unreliable clients.

It defines:

* request deduplication semantics
* asynchronous execution model
* operation tracking
* trace propagation

It does NOT define:

* business domains
* data models
* resource semantics beyond operations
* storage implementations

---

## 2. Identifier Model

All request metadata is classified into four categories.

### 2.1 Idempotency Key

> A client-generated token identifying a logical request execution.

* Scope: single logical command execution
* Purpose: duplicate suppression
* Must NOT represent business identity
* Must NOT be reused across distinct operations

---

### 2.2 Operation Identifier

> A server-generated identifier representing an asynchronous execution instance.

* Scope: one server-side execution unit
* Purpose: execution tracking and polling
* Must survive failures and restarts
* Must NOT be used as business identifier

---

### 2.3 Correlation Identifier

> A trace identifier propagated across all services involved in processing a request.

* Scope: end-to-end request flow
* Purpose: observability and diagnostics
* Must NOT affect execution semantics

---

### 2.4 Request Execution Record (conceptual)

> A durable server-side record linking request metadata to execution state.

Contains:

* Idempotency Key
* Operation ID (if async)
* Request hash
* Execution state
* Response metadata

---

## 3. HTTP Headers

### 3.1 Idempotency-Key

Client-provided.

Used for detecting duplicate logical requests.

Requests with identical Idempotency-Key:

* MUST NOT be executed more than once
* MUST return identical operation outcome

---

### 3.2 Correlation-Id

Client- or gateway-provided.

Must be propagated unchanged across all services.

Must not influence request execution or response generation.

---

## 4. Execution Model

## 4.1 Synchronous Execution

If execution completes within service constraints:

```http id="k9q1lm"
200 OK
or
201 Created
```

No Operation ID is required.

---

## 4.2 Asynchronous Execution

If execution cannot complete within response budget:

```http id="q8x7za"
202 Accepted
Location: /operations/{operationId}
```

The server MUST:

* persist execution state BEFORE returning response
* associate Idempotency-Key with Operation ID

---

## 5. Operation Resource

## 5.1 Operation State Machine

```text id="m2q7tv"
Pending → Running → Completed
                    ↘ Failed
```

State transitions MUST be monotonic.

---

## 5.2 Operation Retrieval

```http id="x1v9nd"
GET /operations/{operationId}
```

Returns current execution state.

---

## 6. Idempotency Semantics

## 6.1 Duplicate Request Handling

If a request is received with an existing Idempotency-Key:

* Server MUST return the original Operation ID (if async)
* Server MUST NOT re-execute the operation
* Server MUST ensure response consistency

---

## 6.2 Request Mismatch Handling

If same Idempotency-Key is used with different request payload:

* Server MUST reject request
* Server MUST NOT execute operation

---

## 7. Failure Model Requirements

## 7.1 Crash Safety Requirement

A system implementing this RFC MUST ensure:

> Any request acknowledged with 202 MUST remain observable via Operation ID after full system restart.

This implies:

* Operation state MUST be durably persisted
* Idempotency mapping MUST be durable or reconstructible

---

## 7.2 Cache Constraint

Ephemeral stores (e.g., Redis):

* MAY be used for acceleration
* MUST NOT be the source of truth for:

  * operation existence
  * idempotency correctness
  * execution state

---

## 8. Terminal / Client Responsibilities (generic, no domain)

Clients MUST:

* persist Idempotency-Key for each logical request
* persist Operation ID after receiving 202
* retry requests using the same Idempotency-Key after failure
* poll Operation endpoint until terminal state

Clients MUST NOT:

* infer completion from 202
* generate multiple Idempotency-Keys for the same logical operation
* discard Operation ID before completion

---

## 9. Key Design Principle

> HTTP request execution is split into three independent concerns:

| Concern               | Mechanism       |
| --------------------- | --------------- |
| Duplicate suppression | Idempotency-Key |
| Execution tracking    | Operation ID    |
| Observability         | Correlation ID  |

These MUST remain independent and MUST NOT be conflated.

---

## 10. Non-Goals

This specification explicitly does NOT define:

* domain identifiers
* business transaction models
* financial or industrial semantics
* session or aggregate concepts

---

