# RFC: Idempotent Command Execution Protocol

**Version:** 0.1  
**Status:** Draft  

---

# 1. Introduction

This specification defines an HTTP-based execution protocol for reliable command processing in distributed systems with unreliable clients and intermittent connectivity.

The protocol provides:

- request deduplication  
- durable server-side execution tracking  
- asynchronous execution support  
- recovery from client and server failures  

This specification explicitly defines a model that does not require a dedicated Operations endpoint.

---

# 2. Normative Language

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**, **SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **MAY**, and **OPTIONAL** in this document are to be interpreted as described in RFC 2119 and RFC 8174, and only when they appear in all capitals.

For consistency, this specification uses **MUST**/**MUST NOT** for absolute requirements rather than the equivalent **SHALL**/**SHALL NOT**.

---

# 3. Scope

## 3.1 In Scope

- HTTP request execution semantics  
- Idempotency-based request deduplication  
- Asynchronous execution via `202 Accepted`  
- Server-side execution state persistence requirements  
- Retry and recovery semantics  

## 3.2 Out of Scope

- Domain-specific models  
- Business transaction semantics  
- Storage implementation details  
- Database schema design  
- Dedicated operation query endpoints  

---

# 4. Core Concepts

## 4.1 Execution Record

An Execution Record is a server-side durable representation of a command execution instance.

It contains:

- Idempotency-Key  
- Request hash  
- Execution state  
- Response data (final or partial)  
- Timestamps  

---

## 4.2 Execution State

An Execution Record MUST transition through the following states:

- Pending  
- Running  
- Completed  
- Failed  

State transitions MUST be monotonic and persisted durably.

---

## 4.3 Idempotency-Key

The Idempotency-Key is a client-generated identifier that uniquely represents a logical command execution.

It is used to ensure that repeated requests do not result in multiple executions.

> **Reference:** The Idempotency-Key request header is described by MDN ([Idempotency-Key][mdn-idempotency-key]) and specified by the IETF HTTPAPI draft ([The Idempotency-Key HTTP Header Field][ietf-idempotency-key]). This specification adopts that definition and constrains it for command execution.

---

## 4.4 Correlation-Id

The Correlation-Id is a trace identifier used for observability.

It MUST be propagated across service boundaries and MUST NOT influence execution semantics.

---

# 5. HTTP Headers

## 5.1 Idempotency-Key

The client MUST include an Idempotency-Key in all non-idempotent requests (POST and PATCH).

> **Note on method semantics:** Per HTTP semantics ([RFC 9110 §9.2.2][rfc9110-idempotent]), POST and PATCH are not idempotent. The Idempotency-Key header is therefore intended to add idempotency to these methods — most notably repeated POST requests. See MDN ([Idempotency-Key][mdn-idempotency-key]) for the general header semantics.

### Requirements

- MUST be unique per logical command execution  
- MUST be reused for retries of the same command  
- MUST NOT be reused for different commands  

---

## 5.2 Correlation-Id

The client MAY include a Correlation-Id.

If present:

- MUST be propagated unchanged by all services  
- MUST NOT influence execution outcome  

---

# 6. Execution Model

## 6.1 Synchronous Execution

If a command completes within the server's response budget, the server returns a terminal success response and includes the final result in the body:

```http
HTTP/1.1 200 OK
```

or, when a new resource is created:

```http
HTTP/1.1 201 Created
```

No Execution Record retrieval is required by the client.

---

## 6.2 Asynchronous Execution

If a command cannot be completed within the response budget:

```http
202 Accepted
```

### Requirements

The server:

- MUST persist the Execution Record BEFORE returning the response  
- MUST associate the Idempotency-Key with the Execution Record  
- MUST ensure the Execution Record is recoverable after failure  

---

# 7. Idempotency Semantics

## 7.1 Duplicate Request Handling

If a request is received with an existing Idempotency-Key:

- The server MUST NOT re-execute the command  
- The server MUST return the current Execution Record state  
- The server MUST ensure response consistency across retries  

While the Execution Record is still `Pending` or `Running`, the server MUST return `202 Accepted` for the replayed request. Once the record reaches `Completed` or `Failed`, the server MUST return the terminal response.

---

## 7.2 Request Payload Consistency

If a request is received with an existing Idempotency-Key but a different payload:

- The server MUST reject the request with `422 Unprocessable Content` (a `409 Conflict` MAY be used where more appropriate)  
- The server MUST NOT modify existing Execution Records  

---

# 8. Recovery Model

## 8.1 Client Failure Recovery

If a client loses the response to a request:

- The client MUST retry the original request with the same Idempotency-Key  
- The server MUST return the existing Execution Record state  

---

## 8.2 Server Failure Recovery

If the server crashes after issuing a `202 Accepted` response:

- The Execution Record MUST be recoverable from durable storage  
- The execution MUST resume or remain queryable via retrying the same request  

---

## 8.3 Network Uncertainty

The client MUST treat the following as ambiguous outcomes:

- timeout  
- connection loss  
- missing response  

In all cases, the client MUST retry using the same Idempotency-Key.

---

# 9. Polling and Status Retrieval

This specification does not define a dedicated status endpoint.

Instead, clients MUST use the idempotent POST request itself as the mechanism for status retrieval.

### Behavior

A repeated request with the same Idempotency-Key:

- MUST NOT trigger re-execution  
- MUST return the current Execution Record state or final response  

> **Reference:** This repeated POST behavior follows the idempotency guarantees described for the Idempotency-Key header by MDN ([Idempotency-Key][mdn-idempotency-key]) and the IETF HTTPAPI draft ([The Idempotency-Key HTTP Header Field][ietf-idempotency-key]): a server that has already processed a key returns the original outcome instead of re-processing the request.

---

# 10. Role of Persistent Storage

## 10.1 Durable Storage Requirement

The server MUST persist Execution Records in durable storage before acknowledging a request with `202 Accepted`.

Failure to persist MUST result in request failure (no acknowledgment).

---

## 10.2 Ephemeral Storage (Optional)

Ephemeral storage (e.g. caches) MAY be used for optimization but:

- MUST NOT be the source of truth for execution state  
- MUST NOT determine idempotency correctness  
- MUST NOT be required for recovery  

---

## 10.3 Idempotency-Key Retention

The server MUST retain Execution Records for a defined retention window sufficient to cover the client's maximum expected retry period.

- The retention window SHOULD be published or otherwise made known to clients  
- After the retention window expires, the server MAY discard the Execution Record  
- A request replayed with an expired Idempotency-Key MAY be treated as a new command execution  

---

# 11. Failure Semantics

## 11.1 Crash Before Persistence

- No `202 Accepted` MAY be returned  
- Client retries safely  

---

## 11.2 Crash After Persistence

- Execution Record MUST remain available  
- Client retries return same state  

---

## 11.3 Cache Loss

Cache loss MUST NOT affect correctness, only performance.

---

# 12. Design Principles

## 12.1 Execution Uniqueness

> Each Idempotency-Key corresponds to exactly one Execution Record.

---

## 12.2 Execution Persistence First

> A server MUST NOT acknowledge acceptance (`202 Accepted`) without durable persistence of execution state.

---

## 12.3 Stateless Client Assumption

Clients MUST NOT rely on server-side session continuity.

All recovery is driven by Idempotency-Key replay.

---

## 12.4 No Operation Resource Requirement

This protocol explicitly does NOT require:

- Operation endpoints  
- Operation identifiers exposed to clients  
- Separate polling APIs  

All state retrieval is performed through idempotent request replay.

---

# 13. Security Considerations

- Idempotency-Key reuse across different payloads MUST be rejected  
- Execution Records MUST be protected against unauthorized replay  
- Correlation-Id MUST NOT be used for authorization decisions  

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

# 16. References

- [MDN — Idempotency-Key][mdn-idempotency-key]  
- [MDN — Idempotent request methods (glossary)][mdn-idempotent]  
- [IETF HTTPAPI — The Idempotency-Key HTTP Header Field][ietf-idempotency-key]  
- [RFC 9110 — HTTP Semantics, §9.2.2 Idempotent Methods][rfc9110-idempotent]  
- [RFC 2119 — Key words for use in RFCs to Indicate Requirement Levels][rfc2119]  
- [RFC 8174 — Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words][rfc8174]  

[mdn-idempotency-key]: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Idempotency-Key
[mdn-idempotent]: https://developer.mozilla.org/en-US/docs/Glossary/Idempotent
[ietf-idempotency-key]: https://datatracker.ietf.org/doc/draft-ietf-httpapi-idempotency-key-header/
[rfc9110-idempotent]: https://www.rfc-editor.org/rfc/rfc9110#section-9.2.2
[rfc2119]: https://www.rfc-editor.org/rfc/rfc2119
[rfc8174]: https://www.rfc-editor.org/rfc/rfc8174

---
