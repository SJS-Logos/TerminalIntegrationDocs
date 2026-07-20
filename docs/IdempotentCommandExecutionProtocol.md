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

An Execution Record is a server-side representation of a command execution instance.

It contains:

- Idempotency-Key  
- Request hash  
- Execution state  
- Response data (final or partial)  
- Timestamps  

An Execution Record becomes **durable** only once the server has taken ownership of the command (Section 4.5). Prior to that point — for example while a request has only been acknowledged with `202 Accepted` — the record MAY be held in non-durable host storage and MAY be lost on server restart or power cycle.

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

> **Generation guidance (non-normative):** This specification does not constrain how the Idempotency-Key is generated. Practical trade-offs — SQL Server storage locality, generation cost, boot-time entropy on constrained devices, and composite/monotonic key strategies — are discussed in the companion document [Idempotency-Key Generation Considerations][keygen].

---

## 4.4 Correlation-Id

The Correlation-Id is a trace identifier used for observability. On the wire it is carried by the organization's custom `X-Correlation-Id` header, defined by the `AiO.Constants` library (`HeaderNames.CorrelationId`). It is not an IANA-registered or MDN-documented standard header; the standardized equivalent for distributed tracing is W3C Trace Context ([`traceparent`/`tracestate`][w3c-trace-context]).

It MUST be propagated across service boundaries and MUST NOT influence execution semantics.

> **Note on transport identifiers and domain data:** The Correlation-Id is defined here alongside the Idempotency-Key because both are transport-level identifiers that a service MAY surface into its domain. While the Correlation-Id itself is observability-only and MUST NOT drive execution, transport identifiers in general may legitimately be mapped into domain inputs — for example, the `Idempotency-Key` may be carried into the Domain as an explicit command input (Section 10.4), and a Correlation-Id MAY be copied into domain data where a service chooses to record it. Such mapping is an Incoming Implementation concern (AP-003 Mapping into a Contract Model) and does not change the rule that the Correlation-Id has no effect on idempotency or execution outcome.

---

## 4.5 Ownership

**Ownership** is the point at which the server has durably accepted responsibility for the data associated with an Idempotency-Key.

- The server takes ownership only when it returns a terminal success response: `200 OK` or `201 Created`. At that point the associated data is durably persisted and MUST survive server restart.
- A `202 Accepted` response does **not** transfer ownership. It is a best-effort acknowledgment that the request was received; the associated Execution Record MAY reside in non-durable host storage and MAY be lost on power cycle.

Because ownership is confirmed only by `200`/`201`, the default client behavior is to **retry the request with the same Idempotency-Key until it observes a terminal `200`/`201` response** (Sections 8 and 9). A retried request whose earlier `202`-acknowledged record did not survive MAY be treated by the server as a new command execution.

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

The client MAY include a Correlation-Id, carried by the custom `X-Correlation-Id` header (`AiO.Constants` `HeaderNames.CorrelationId`). This is a de-facto, organization-defined header rather than a standardized one; services requiring standards-based tracing SHOULD additionally support W3C Trace Context ([`traceparent`/`tracestate`][w3c-trace-context]).

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

A `200`/`201` response signals that the server has taken **ownership** (Section 4.5): the result is durably persisted and no Execution Record retrieval is required by the client.

---

## 6.2 Asynchronous Execution

If a command cannot be completed within the response budget:

```http
202 Accepted
```

A `202 Accepted` is a **best-effort acknowledgment** and does **not** transfer ownership (Section 4.5). Because the host cannot guarantee durability at this stage, the Execution Record MAY be lost on server restart or power cycle.

### Requirements

The server:

- MUST associate the Idempotency-Key with the Execution Record while the record exists  
- SHOULD persist the Execution Record on a best-effort basis to aid recovery, but MUST NOT rely on that persistence being durable  
- MUST take durable ownership of the data before, and only signal it by, returning a terminal `200`/`201` response  

### Client obligation

The client MUST NOT treat a `202 Accepted` as confirmation that the command result is durably stored. The client MUST continue to retry with the same Idempotency-Key until it observes a terminal `200`/`201` response (Sections 8 and 9).

---

# 7. Idempotency Semantics

## 7.1 Duplicate Request Handling

If a request is received with an existing, still-available Execution Record for the Idempotency-Key:

- The server MUST NOT re-execute the command  
- The server MUST return the current Execution Record state  
- The server MUST ensure response consistency across retries  

While the Execution Record is still `Pending` or `Running`, the server MUST return `202 Accepted` for the replayed request. Once ownership has been taken and the record reaches `Completed` or `Failed`, the server MUST return the terminal `200`/`201` response.

Because a `202`-acknowledged Execution Record is not durable (Sections 4.5 and 6.2), a replayed request MAY arrive after its earlier record has been lost. In that case the server has not taken ownership, and it MAY treat the request as a new command execution. This is safe precisely because the client identifies the command by its Idempotency-Key: the command is either resumed from a surviving record or executed anew, and ownership is confirmed only by a terminal `200`/`201`.

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

- The Execution Record MAY have been lost, because a `202` acknowledgment is non-durable and does not transfer ownership (Sections 4.5 and 6.2)  
- On retry with the same Idempotency-Key, the server MUST resume from a surviving record if one exists, or MAY treat the request as a new command execution if the record did not survive  
- The client MUST continue retrying until it observes a terminal `200`/`201` response, at which point ownership has been durably taken

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

- MUST NOT trigger re-execution while a surviving Execution Record exists  
- MUST return the current Execution Record state, or the terminal `200`/`201` response once ownership has been taken  

Because a `202`-acknowledged record is non-durable (Section 4.5), a replay MAY find no record and be treated as a new execution. The terminal `200`/`201` response is therefore the definitive signal that the command has completed and its result is durably owned by the server.

> **Reference:** This repeated POST behavior follows the idempotency guarantees described for the Idempotency-Key header by MDN ([Idempotency-Key][mdn-idempotency-key]) and the IETF HTTPAPI draft ([The Idempotency-Key HTTP Header Field][ietf-idempotency-key]): a server that has already processed a key returns the original outcome instead of re-processing the request.

---

# 10. Role of Persistent Storage

## 10.1 Durable Storage Requirement

The server MUST persist Execution Records in durable storage before taking **ownership** of a command — that is, before returning a terminal `200 OK` or `201 Created` response.

A `202 Accepted` response does NOT require durable persistence; hosts that cannot guarantee durability MAY hold the interim Execution Record in non-durable storage that is lost on power cycle (Sections 4.5 and 6.2). Failure to durably persist before a terminal `200`/`201` MUST prevent that terminal response from being returned.

---

## 10.2 Ephemeral Storage (Optional)

Ephemeral storage (e.g. caches) MAY be used for optimization but:

- MUST NOT be the source of truth for execution state  
- MUST NOT determine idempotency correctness  
- MUST NOT be required for recovery  

---

## 10.3 Idempotency-Key Retention

The server MUST retain owned (durably persisted) Execution Records for a defined retention window sufficient to cover the client's maximum expected retry period.

- The retention window SHOULD be published or otherwise made known to clients  
- After the retention window expires, the server MAY discard the Execution Record  
- A request replayed with an expired Idempotency-Key MAY be treated as a new command execution  

---

## 10.4 Placement of Idempotency Handling

> **Terminology alignment:** This section aligns with the logical model of AP-001 [ap-001] and the incoming-implementation structure of AP-003 [ap-003]. The **Incoming Implementation** (the AP-003 *Transport Endpoint* within an HTTP *Host Unit*) is referred to below as the *transport layer*; the **Domain** (AP-001 §6.3), coordinated by the **Application**, is referred to as the *domain layer*. "Moving the key into the DTO" is, in AP-003 terms, a **Mapping** of the `Idempotency-Key` into an Application **Contract Model** (Request Model).

Idempotency enforcement MAY be handled at either of two layers, and this specification does not mandate a single placement:

- **Transport layer (Incoming Implementation)** — the Transport Endpoint inspects the incoming `Idempotency-Key` header and short-circuits duplicate requests before they reach the domain.  
- **Domain layer** — the Domain owns idempotency enforcement as part of command execution.

If idempotency is enforced in the **domain**, the Incoming Implementation MUST map the transport-level `Idempotency-Key` into an Application Contract Model (per AP-003 Mapping) so that the domain receives the key as an explicit input. The domain MUST NOT depend on transport-specific constructs to obtain the key (AP-001 §6.3).

If idempotency is enforced in the **transport layer**, the Incoming Implementation MUST NOT allow a duplicate command to reach the domain, and the resulting Execution Record state MUST remain consistent with the semantics defined in Sections 7 and 10. Consistent with AP-001 §6.1 and AP-003, the Incoming Implementation MUST NOT make business decisions; idempotency deduplication is treated here as a technical operation, not a business outcome.

---

## 10.5 Storage Expectations per Layer

Regardless of placement, idempotency correctness MUST ultimately rest on persisted infrastructure (Section 10.1). The two layers MAY, however, use different storage strategies:

- **Transport layer (Incoming Implementation)** — MAY participate in idempotency in one of two variations:
  - *Authoritative variation* — the Incoming Implementation maintains its own durable idempotency store that records the `Idempotency-Key` and the recorded response, and resolves duplicates at the edge without invoking the domain. In this variation the store MUST meet the durability requirements of Section 10.1.
  - *Optimization variation* — the Incoming Implementation short-circuits obvious duplicates while the domain remains authoritative. It MAY use any storage that fits (volatile or non-volatile); the store remains an optimization regardless of its durability, and per Section 10.2 it MUST NOT be the source of truth and MUST NOT be required for recovery. Because idempotency correctness is enforced by the domain in this variation, the Incoming Implementation MUST forward the `Idempotency-Key` to the domain (per Section 10.4).
- **Domain layer** — MUST use durable, persisted storage for the Execution Record so that idempotency survives process and server failures.

Where both layers persist independently, each layer's store is authoritative for the idempotency decision made at that layer. The layers MUST remain consistent with the duplicate-handling and payload-consistency semantics defined in Sections 7 and 10; in particular, a duplicate resolved in the transport layer MUST yield a response consistent with the domain's Execution Record state.

> **Informative note (non-normative):** This two-layer arrangement is not a distinct, standardized "dual-layer idempotency" model, and it does not require a cache. It is a composition of the `Idempotency-Key` request contract ([IETF HTTPAPI draft][ietf-idempotency-key], [MDN][mdn-idempotency-key]) with an idempotency store maintained at more than one layer. This mirrors the pattern observed in Stripe's client/server idempotency handling ([Stripe's idempotent requests][stripe-idempotency]), where keys and recorded responses are persisted durably at the API (node) layer independently of any downstream domain persistence. See also the [AWS Builders' Library on idempotent APIs][aws-idempotency] for durable key-based idempotency guidance.

> **Informative note (non-normative):** A generic platform-level layer (for example a shared service backed by Redis, with a TTL aligned to the Section 10.3 retention window) is a common way to implement the transport-layer store. Note that Redis persistence is durable-ish rather than transactional: RDB snapshots may lose writes since the last snapshot; AOF with `appendfsync everysec` may lose up to roughly one second of writes; and asynchronous replication may lose acknowledged writes on failover unless stronger guarantees (e.g. `WAIT`) are configured. Such a layer is well suited to the optimization variation, but MUST NOT be treated as authoritative unless its configured durability genuinely satisfies Section 10.1.

---

# 11. Failure Semantics

## 11.1 Crash Before Ownership

- A `200`/`201` MUST NOT be returned until ownership is durably taken  
- A prior `202 Accepted` (if any) MAY be lost  
- Client retries safely with the same Idempotency-Key  

---

## 11.2 Crash After Ownership

- Once `200`/`201` has been returned, the Execution Record MUST remain durably available  
- Client retries return the same terminal state  

---

## 11.3 Cache Loss

Cache loss MUST NOT affect correctness, only performance.

---

# 12. Design Principles

## 12.1 Execution Uniqueness

> Each Idempotency-Key corresponds to exactly one Execution Record.

---

## 12.2 Ownership Persistence First

> A server MUST NOT take ownership (return `200 OK` or `201 Created`) without durable persistence of execution state. A `202 Accepted` conveys no such guarantee.

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
| Durable ownership     | Terminal `200`/`201`   | Server         |
| Retry recovery        | Idempotent POST replay | Client         |
| Observability         | Correlation-Id         | Infrastructure |

---

# 15. Final Principle

> In this model, the idempotent request itself is the handle to both command execution and execution state retrieval. A `202 Accepted` is only a best-effort acknowledgment; a terminal `200`/`201` is the sole confirmation that the server has taken durable ownership of the command's outcome.

---

# 16. References

- [MDN — Idempotency-Key][mdn-idempotency-key]  
- [MDN — Idempotent request methods (glossary)][mdn-idempotent]  
- [IETF HTTPAPI — The Idempotency-Key HTTP Header Field][ietf-idempotency-key]  
- [RFC 9110 — HTTP Semantics, §9.2.2 Idempotent Methods][rfc9110-idempotent]  
- [RFC 2119 — Key words for use in RFCs to Indicate Requirement Levels][rfc2119]  
- [RFC 8174 — Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words][rfc8174]  
- [W3C — Trace Context (`traceparent`/`tracestate`)][w3c-trace-context]  

**Informative (non-normative):**

- [Stripe API — Idempotent requests][stripe-idempotency]  
- [AWS Builders' Library — Making retries safe with idempotent APIs][aws-idempotency]  

**Related internal specifications:**

- [AP-001 — Architectural Principles][ap-001]  
- [AP-003 — Incoming Implementations][ap-003]  
- [Idempotency-Key Generation Considerations (companion)][keygen]  

[mdn-idempotency-key]: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Idempotency-Key
[mdn-idempotent]: https://developer.mozilla.org/en-US/docs/Glossary/Idempotent
[ietf-idempotency-key]: https://datatracker.ietf.org/doc/draft-ietf-httpapi-idempotency-key-header/
[rfc9110-idempotent]: https://www.rfc-editor.org/rfc/rfc9110#section-9.2.2
[rfc2119]: https://www.rfc-editor.org/rfc/rfc2119
[rfc8174]: https://www.rfc-editor.org/rfc/rfc8174
[w3c-trace-context]: https://www.w3.org/TR/trace-context/
[stripe-idempotency]: https://docs.stripe.com/api/idempotent_requests
[aws-idempotency]: https://aws.amazon.com/builders-library/making-retries-safe-with-idempotent-APIs/
[ap-001]: ./AP-001.md
[ap-003]: ./AP-003.md
[keygen]: ./IdempotencyKeyGeneration.md

---
