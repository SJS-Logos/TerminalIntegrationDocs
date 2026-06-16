# RFC: HTTP Request Execution, Idempotency, and Operation Lifecycle Protocol

## 1. Abstract

This specification defines a transport-layer protocol for reliable execution of client-initiated operations over HTTP in environments with unreliable connectivity (e.g., embedded terminals).

It introduces three identifiers:

* Idempotency Key (request deduplication)
* Operation ID (server-side execution tracking)
* Correlation ID (distributed tracing)

It also defines an Operation resource model and strict rules for 202 Accepted semantics to ensure crash safety and recoverability.

---

## 2. Terminology

### 2.1 Operation

A server-side execution instance representing a single logical unit of work initiated by a client request.

### 2.2 Terminal State

A state in which an Operation is no longer progressing:

* Completed
* Failed
* Cancelled

### 2.3 Durable Store

A persistent, crash-resilient storage system (e.g., database or event log).

### 2.4 Ephemeral Store

A non-durable cache (e.g., Redis), which may lose data at any time.

---

## 3. HTTP Headers

### 3.1 Idempotency-Key

**Header:** `Idempotency-Key`

* Supplied by: Client
* Scope: Single logical request execution
* Lifetime: Until Operation reaches terminal state or key expires per retention policy

#### Semantics

The Idempotency-Key uniquely identifies a logical command invocation.

Requests with the same Idempotency-Key MUST NOT result in multiple executions.

---

### 3.2 Correlation-Id

**Header:** `Correlation-Id`

* Supplied by: Client or Gateway
* Scope: End-to-end request tracing
* Lifetime: Single request flow across services

#### Semantics

Correlation-Id is used exclusively for observability and MUST NOT influence execution semantics.

---

## 4. Operation Resource Model

### 4.1 Operation Identifier

The server SHALL generate an OperationId for each accepted asynchronous operation.

* Globally unique within system scope
* Immutable
* Not meaningful to business domain

---

### 4.2 Operation State Machine

An Operation SHALL transition through the following states:

```text
Pending → Running → Completed
                    ↘ Failed
                    ↘ Cancelled
```

State transitions MUST be monotonic and persisted.

---

### 4.3 Operation Persistence Requirement

All Operations MUST be written to Durable Store BEFORE any HTTP response is returned.

Failure to persist MUST result in request failure (no 2xx response).

---

## 5. HTTP Semantics

### 5.1 Synchronous Execution

If execution completes within service time budget:

```http
200 OK
or
201 Created
```

No Operation resource is required.

---

### 5.2 Asynchronous Execution

If execution cannot complete synchronously:

```http
202 Accepted
Location: /operations/{operationId}
```

Response MUST be returned only after:

* Operation is durably persisted
* Initial state is recorded (at minimum: Pending or Running)

---

### 5.3 202 Accepted Semantics

A 202 response constitutes:

> A durable transfer of execution responsibility from client to server.

After 202 is returned:

* The server MUST ensure eventual completion or failure is observable via Operation resource
* The Operation MUST survive server restart, process failure, and cache loss

---

## 6. Operation Endpoint

### 6.1 Retrieve Operation

```http
GET /operations/{operationId}
```

Response:

```json
{
  "operationId": "12345",
  "state": "Running"
}
```

or terminal state:

```json
{
  "operationId": "12345",
  "state": "Completed"
}
```

---

### 6.2 Idempotent Retry Behavior

If a client repeats the original request with the same Idempotency-Key:

* The server MUST return the same OperationId
* The server MUST return the same 202 response metadata
* The operation MUST NOT be executed again

---

## 7. Storage Architecture Requirements

### 7.1 Durable Store (Mandatory)

Must store:

* OperationId
* IdempotencyKey
* RequestHash
* State
* Timestamps

Must survive:

* power failure
* process crash
* node restart

---

### 7.2 Ephemeral Store (Optional)

May store:

* Idempotency-Key → OperationId mapping
* Cached Operation state
* Fast lookup indexes

Must NOT be the source of truth.

---

## 8. Failure Modes and Recovery

### 8.1 Response Loss After 202

If HTTP response is lost after server-side persistence:

* Client retries request with same Idempotency-Key
* Server returns existing OperationId
* Client resumes polling Operation resource

No duplicate execution occurs.

---

### 8.2 Server Crash After 202

If server crashes after returning 202:

* Operation MUST be recoverable from Durable Store
* Operation state MUST be queryable via Operation endpoint

---

### 8.3 Cache Loss (e.g., Redis flush)

Cache loss MUST NOT affect:

* idempotency correctness
* operation existence
* operation state

Cache is strictly a performance optimization.

---

## 9. Client Responsibilities (Embedded Terminals)

Clients MUST:

1. Persist Idempotency-Key for each logical command
2. Persist OperationId after receiving 202
3. Poll Operation endpoint until terminal state is reached
4. Retry original request using same Idempotency-Key if response is lost or ambiguous
5. Treat 202 as acceptance of responsibility transfer

Clients MUST NOT:

* assume completion from 202
* generate multiple Idempotency-Keys for the same logical action
* discard OperationId before terminal state

---

## 10. Design Principle

> Idempotency ensures single execution.
> Operation ensures observability of execution.
> Correlation ensures traceability of execution.

These concerns are intentionally orthogonal and MUST NOT be merged.

---

## 11. Security and Integrity Considerations

* RequestHash SHOULD be stored alongside Idempotency-Key to detect semantic changes
* Mismatched requests with same Idempotency-Key MUST be rejected
* Replay attacks MUST be mitigated via Idempotency-Key uniqueness enforcement within retention window

---

## 12. Non-Goals

This specification does not define:

* business transaction semantics (e.g., payments, parking rules)
* domain identifiers (e.g., SessionId)
* settlement or reconciliation logic
