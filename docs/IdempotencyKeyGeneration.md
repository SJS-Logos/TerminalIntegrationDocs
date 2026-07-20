# RFC: Idempotency-Key Generation Considerations

**Version:** 0.1  
**Status:** Draft  
**Companion to:** [Idempotent Command Execution Protocol](./IdempotentCommandExecutionProtocol.md)

---

# 1. Introduction

This document is a non-normative companion to the *Idempotent Command Execution Protocol* (the "Protocol"). The Protocol requires a client-generated `Idempotency-Key` (Protocol §4.3, §5.1) but deliberately does not constrain how that key is generated.

This companion explores the practical trade-offs of key generation, particularly for constrained clients (payment terminals and similar embedded devices) and for server-side storage. Both **SQL Server** (used by the majority of current products) and **PostgreSQL** (a forward-looking target) are covered, since the estate is expected to run a mix of the two during transition. Section 7 additionally covers a related transport-level cost — the bandwidth of retrying commands under the Protocol's non-durable `202` model — because it directly shapes how a terminal reuses the `Idempotency-Key`; the normative rule for that behavior lives in the Protocol (§9.1). This document provides guidance and references; it does not alter the normative requirements of the Protocol.

---

# 2. Normative Language

This document is primarily informative. Where **MUST**, **SHOULD**, or **MAY** appear in all capitals, they are to be interpreted as described in RFC 2119 and RFC 8174, and apply only to the recommendations in this companion — not to the Protocol itself.

---

# 3. Requirements Recap

An Idempotency-Key (Protocol §5.1) MUST:

- be unique per logical command execution,
- be reused for retries of the same command,
- not be reused for different commands.

Any generation strategy discussed here MUST preserve these properties. The additional concerns below — storage locality, generation cost, and entropy — are optimizations layered on top of these invariants.

---

# 4. Server-Side Storage Locality

The cost of a random UUID/GUID key depends heavily on how the database physically organizes rows. The two engines in the estate differ fundamentally: SQL Server orders the table by a **clustered index**, while PostgreSQL stores rows in an unordered **heap**. The same random key is therefore far more expensive on SQL Server than on PostgreSQL.

## 4.1 SQL Server

### Problem

A random `uniqueidentifier` (for example `NEWID()`) used as the **clustered** primary key causes inserts to land at random positions in the B-tree, producing page splits, index fragmentation, and poor cache locality. Because the clustering key is duplicated into every non-clustered index, a wide random key also inflates all secondary indexes.

This is a property of the **clustered index**, not of the GUID value itself; a GUID stored in a non-clustered unique index does not cause the same insert-ordering problem.

### Options

- **Narrow surrogate + unique non-clustered index (recommended default).** Use a `BIGINT IDENTITY` or a sequence as the clustered key, and store the `Idempotency-Key` in a `UNIQUE` non-clustered index. This preserves insert locality while still enforcing idempotency uniqueness.
- **Time-ordered identifiers.** `NEWSEQUENTIALID()` (server-generated) or client-generated sequential/COMB GUIDs place a time component in the high-order bytes (respecting SQL Server's GUID sort order), avoiding random-insert fragmentation. Note `NEWSEQUENTIALID()` cannot be client-generated and leaks host/timing information.
- **UUIDv7.** RFC 9562 (which obsoletes RFC 4122) standardizes time-ordered UUIDs; a good option when a single client-generated ordered identifier is desired. Store it in a `uniqueidentifier` column and be aware of SQL Server's non-byte-order GUID sort.

### Guidance

- Implementations SHOULD NOT use a random GUID as the clustered key on SQL Server.
- Implementations SHOULD prefer either a narrow surrogate clustered key with a unique non-clustered index on the `Idempotency-Key`, or a time-ordered identifier (UUIDv7 / sequential GUID).

## 4.2 PostgreSQL

### Problem

PostgreSQL stores table rows in a **heap**; a primary key is an ordinary B-tree index beside the heap rather than a clustered index that dictates physical row order. As a result, a random UUID primary key does **not** cause the page-split/fragmentation cascade seen on clustered-index databases, and the key is **not** duplicated into every secondary index as a wide clustering key.

Random UUIDv4 keys are nonetheless not free. Poor B-tree insert locality keeps the index's insertion points scattered rather than hot, which increases buffer-cache misses, index bloat, and **write-ahead log (WAL) amplification** (more full-page images touched per insert). At high insert rates this is a measurable cost, though far less severe than on SQL Server's clustered index.

The native `uuid` type is a compact 16-byte value (not a 36-character string), so storage and comparison are efficient; the concern is insert ordering, not the value size.

### Options

- **`uuid` primary key (acceptable default).** Unlike SQL Server, a `uuid` primary key is a reasonable default in PostgreSQL because there is no clustered index to fragment. A random UUIDv4 works, at the cost of the WAL/cache locality overhead described above.
- **Time-ordered identifiers (preferred).** UUIDv7 (RFC 9562) places a timestamp in the high-order bytes, keeping B-tree inserts on the right-hand (hot) side of the index. This reduces cache misses and WAL amplification and is the recommended choice for high-throughput tables. ULID and Snowflake offer similar ordering if a non-UUID encoding is acceptable.
- **`CLUSTER` is not a fix.** `CLUSTER` reorders a table once against an index but is not maintained on subsequent inserts, so it cannot substitute for a time-ordered key.
- **Narrow surrogate (optional).** A `bigint` generated-identity primary key with a `UNIQUE` index on the `Idempotency-Key` remains a valid option, but in PostgreSQL it is an optimization rather than a necessity.

### Guidance

- A `uuid` primary key is acceptable on PostgreSQL; the clustered-key workaround required by SQL Server is not necessary here.
- For high-insert-rate tables, implementations SHOULD prefer a time-ordered identifier (UUIDv7) to improve index locality and reduce WAL amplification.

## 4.3 Portability During Transition

While products span both engines, a **time-ordered identifier (UUIDv7)** is the strategy that performs well on both: it satisfies SQL Server's need to avoid random clustered inserts and improves PostgreSQL's index locality. Choosing UUIDv7 now therefore avoids per-engine schema divergence and eases the SQL Server → PostgreSQL transition. Where a team prefers to keep SQL Server's narrow-surrogate pattern, the `Idempotency-Key` should still live in a unique index so the same logical model ports cleanly to PostgreSQL later.

---

# 5. Client-Side Generation Cost and Entropy

## 5.1 Problem

Constrained clients (payment terminals, embedded devices) may generate keys under two pressures:

- **Cost.** Cryptographic random generation can be expensive on limited CPUs at high transaction rates.
- **Entropy.** Many embedded devices have weak entropy shortly after boot and no hardware RNG. Time-based seeds are predictable because clocks start near a fixed value at power-on. This class of failure is documented in *Mining Your Ps and Qs* (Heninger et al., USENIX Security 2012), which found embedded devices generating predictable keys due to low boot-time entropy.

For idempotency keys the primary risk is not confidentiality but **collision** and **cross-generator duplication**, which violate the uniqueness invariant.

> **Note on the identity namespace:** A physical terminal has a stable **device id** that identifies the hardware, but a single device MAY host several independent applications (for example, one forecourt computer driving several fuel pumps). Each application has a **globally unique application id**. Because the application id is globally unique, it — not the device id — is the correct uniqueness namespace: an identity-scoped scheme MUST scope uniqueness by the tuple **(application id, monotonic component)**. Device id alone is insufficient (co-located applications share it) and, given a globally unique application id, device id is unnecessary for key uniqueness and MAY be omitted (it remains useful only for hardware lookup / observability).

## 5.2 Options

- **Hardware TRNG / secure element.** Many payment terminals include a crypto element for key management; where available it SHOULD be used as the entropy source.
- **Seed-once CSPRNG.** Seed a cryptographically secure PRNG once from a good source, then derive keys cheaply.
- **Identity-scoped keys.** Combine the **globally unique application id** of the app instance with a monotonic component, so uniqueness does not depend on RNG quality. Because the application id is globally unique it is sufficient on its own as the namespace; device id is not required for key uniqueness (see §5.1) and is used only for hardware lookup.

## 5.3 Guidance

- Implementations SHOULD NOT rely on a time-seeded, non-cryptographic PRNG as the sole source of key uniqueness on devices with weak boot-time entropy.
- Implementations SHOULD prefer identity-scoped keys (globally unique application id + monotonic component) or a properly seeded CSPRNG / hardware TRNG. The globally unique application id is the required namespace; device id is not needed for key uniqueness (§5.1).

---

# 6. Composite and Monotonic Keys

## 6.1 Problem

A composite key such as *(application id, counter)* addresses cross-generator uniqueness without global randomness, but introduces two costs:

- **Storage cost.** A wide composite key inflates indexes and, if used as the primary key, worsens the insert-locality overhead of Section 4.
- **Counter durability.** A per-application counter MUST survive reboot; otherwise the application must resync with the server or risk reissuing keys. Persisting every increment is expensive on constrained flash storage. The counter is scoped per **application** (identified by its globally unique application id), so co-located applications on the same device each maintain their own counter (§5.1).

## 6.2 Options

- **Fixed-width encoding.** Hash or pack the composite into a fixed-width value and store it in a non-clustered unique index rather than clustering on it.
- **Block/batch allocation (HiLo).** Reserve a block of N counter values and persist only the high-watermark, bounding writes to once per N increments and tolerating a bounded gap on crash. This is the HiLo pattern used by ORMs such as NHibernate and EF.
- **Time-ordered IDs.** Snowflake, ULID, KSUID, and UUIDv7 combine a timestamp, a node/device identifier, and a small per-tick sequence. They avoid counter resync entirely, requiring only a reliable clock plus a small monotonic guard against clock regression. (KSUID is a segment-style time-ordered ID similar in spirit to ULID.) Where the node identifier is used to disambiguate generators, it MUST be derived from the globally unique application id, not the device id, so co-located applications do not share a node value (§5.1).

## 6.3 Guidance

- If a monotonic counter is used, implementations SHOULD persist it in blocks (HiLo) rather than per-increment, and MUST ensure the persisted high-watermark is durable across reboot. The counter MUST be scoped per application (by its globally unique application id), so co-located applications on the same device do not share a counter (§5.1).
- Implementations MAY prefer time-ordered IDs (UUIDv7 / ULID / Snowflake) to avoid counter-resync complexity, provided the device clock is monotonic or guarded against regression.

---

# 7. Retry Payload Cost under Non-Durable `202`

## 7.1 Problem

Under the Protocol's ownership model, a `202 Accepted` does not transfer ownership and its Execution Record may be lost on power cycle (Protocol §4.5, §6.2, §8.2). The default client behavior is therefore to retry with the same `Idempotency-Key` until it observes a terminal `200`/`201` (Protocol §8, §9). For a payment terminal this means the **full command payload is transmitted at least twice** whenever the server first answers `202`: once for the original request and again on each retry.

The cost that terminal architects are concerned with is **not the key** — the `Idempotency-Key` is a few bytes — but the **repeated transmission of the command body** over a constrained, sometimes metered, link. Key *generation* is cheap by comparison; the expensive part is re-sending the payload for a command the server may already be executing.

This concern is a transport/protocol matter rather than a key-generation property; the normative retry and status-probe contract is defined by the Protocol (§9.1). It is summarized here because it directly shapes how a terminal reuses the `Idempotency-Key` across retries.

## 7.2 Approach

The Protocol defines a **thin status probe** for exactly this situation (Protocol §9.1): after a `202`, the client retries with the `Idempotency-Key` but no command body. If the server still holds a record it answers without a re-upload; if the record was lost it responds `428 Precondition Required` ([RFC 6585][rfc6585]), and only then does the client re-send the full payload. This bounds full-payload retransmission to the actual loss cases rather than every retry.

Complementary, non-normative mitigations:

- **Conditional replay via request hash.** The client sends the `Idempotency-Key` plus the request hash the Protocol already tracks (Protocol §4.1), letting the server request the full body only when it has no matching record. This reuses existing payload-consistency machinery (Protocol §7.2).
- **Payload compression / delta encoding.** Where a thin probe is not available, compressing the command body reduces per-retry cost. This mitigates but does not eliminate the double-send.
- **Longer best-effort retention of `202` records.** Servers MAY hold interim (non-durable) records long enough to cover the common retry window, reducing how often a probe misses. This does not change ownership semantics (still only `200`/`201`); it only reduces the frequency of full re-sends.

### Why `428` and not `404`

A `428 Precondition Required` says "you must re-issue this request carrying more than it did" — precisely the probe-miss case, where the missing precondition is the command body. A `404 Not Found` would be ambiguous against genuine routing or URL errors on a constrained terminal link, and the `422`/`409` payload-mismatch responses (Protocol §7.2) apply only when a record exists but the payload differs. `428` therefore isolates "resend the body" from every other failure mode.

## 7.3 Guidance

- Clients SHOULD, where the server supports it, retry a `202`-acknowledged command with a thin probe (`Idempotency-Key` only) and re-send the full payload only when the server answers `428` (Protocol §9.1).
- The `Idempotency-Key` MUST remain stable across the original request, all thin probes, and any full-payload re-send for the same logical command, so the server can correlate them (Protocol §5.1, §7.1, §9.1).
- Implementations MUST NOT weaken ownership semantics to save bandwidth: a `202` still conveys no durability guarantee, and only a terminal `200`/`201` confirms the result is owned.
- This optimization is transport-level; it does not change how the `Idempotency-Key` itself is generated (Sections 4–6).

---

# 8. Summary of Recommendations

| Concern | Preferred approach |
| --- | --- |
| SQL Server insert locality | Narrow surrogate clustered key + unique non-clustered index, or UUIDv7 |
| PostgreSQL insert locality | `uuid` primary key (acceptable); UUIDv7 preferred for high insert rates |
| Cross-engine portability (transition) | UUIDv7 (performs well on both) |
| Constrained-device entropy | Hardware TRNG / seeded CSPRNG, or identity-scoped keys |
| Cross-generator uniqueness | Globally unique application id + monotonic component (device id not required) |
| Counter durability | HiLo block allocation; durable high-watermark |
| Avoiding counter resync | Time-ordered IDs (UUIDv7 / ULID / Snowflake) |
| Retry payload cost under non-durable `202` | Thin `Idempotency-Key`-only status probe with `428` probe-miss (Protocol \u00a79.1) |

---

# 9. References

- [RFC 9562 — Universally Unique IDentifiers (UUIDs), including v6/v7/v8][rfc9562]  
- [RFC 4122 — A Universally Unique IDentifier (UUID) URN Namespace (obsoleted by RFC 9562)][rfc4122]  
- [RFC 6585 — Additional HTTP Status Codes (including 428 Precondition Required)][rfc6585]  
- [Kimberly Tripp — GUIDs as PRIMARY KEYs and/or the clustering key (SQL Server)][sqlskills-guids]  
- [Jimmy Nilsson — The Cost of GUIDs as Primary Keys (COMB)][comb-guids]  
- [PostgreSQL — UUID Type (`uuid`)][pg-uuid]  
- [PostgreSQL — Physical storage and the heap (`CLUSTER`)][pg-cluster]  
- [Heninger, Durumeric, Wustrow, Halderman — Mining Your Ps and Qs (USENIX Security 2012)][mining-ps-qs]  
- [ULID specification][ulid]  
- [Twitter Snowflake][snowflake]  
- [IETF HTTPAPI — The Idempotency-Key HTTP Header Field][ietf-idempotency-key]  

[rfc9562]: https://www.rfc-editor.org/rfc/rfc9562
[rfc4122]: https://www.rfc-editor.org/rfc/rfc4122
[rfc6585]: https://www.rfc-editor.org/rfc/rfc6585
[pg-uuid]: https://www.postgresql.org/docs/current/datatype-uuid.html
[pg-cluster]: https://www.postgresql.org/docs/current/sql-cluster.html
[sqlskills-guids]: https://www.sqlskills.com/blogs/kimberly/guids-as-primary-keys-andor-the-clustering-key/
[comb-guids]: http://www.informit.com/articles/article.aspx?p=25862
[mining-ps-qs]: https://www.usenix.org/conference/usenixsecurity12/technical-sessions/presentation/heninger
[ulid]: https://github.com/ulid/spec
[snowflake]: https://github.com/twitter-archive/snowflake/tree/snowflake-2010
[ietf-idempotency-key]: https://datatracker.ietf.org/doc/draft-ietf-httpapi-idempotency-key-header/

---
