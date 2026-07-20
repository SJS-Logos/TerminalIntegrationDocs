# RFC: Idempotency-Key Generation Considerations

**Version:** 0.1  
**Status:** Draft  
**Companion to:** [Idempotent Command Execution Protocol](./IdempotentCommandExecutionProtocol.md)

---

# 1. Introduction

This document is a non-normative companion to the *Idempotent Command Execution Protocol* (the "Protocol"). The Protocol requires a client-generated `Idempotency-Key` (Protocol §4.3, §5.1) but deliberately does not constrain how that key is generated.

This companion explores the practical trade-offs of key generation, particularly for constrained clients (payment terminals and similar embedded devices) and for server-side storage. Both **SQL Server** (used by the majority of current products) and **PostgreSQL** (a forward-looking target) are covered, since the estate is expected to run a mix of the two during transition. It provides guidance and references; it does not alter the normative requirements of the Protocol.

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

For idempotency keys the primary risk is not confidentiality but **collision** and **cross-device duplication**, which violate the uniqueness invariant.

## 5.2 Options

- **Hardware TRNG / secure element.** Many payment terminals include a crypto element for key management; where available it SHOULD be used as the entropy source.
- **Seed-once CSPRNG.** Seed a cryptographically secure PRNG once from a good source, then derive keys cheaply.
- **Identity-scoped keys.** Combine a stable device identifier (for example `X-Device-Id`, `AiO.Constants` `HeaderNames.DeviceId`) with a monotonic component so uniqueness does not depend on RNG quality.

## 5.3 Guidance

- Implementations SHOULD NOT rely on a time-seeded, non-cryptographic PRNG as the sole source of key uniqueness on devices with weak boot-time entropy.
- Implementations SHOULD prefer device-scoped keys (device id + monotonic component) or a properly seeded CSPRNG / hardware TRNG.

---

# 6. Composite and Monotonic Keys

## 6.1 Problem

A composite key such as *(device UUID, counter)* addresses cross-device uniqueness without global randomness, but introduces two costs:

- **Storage cost.** A wide composite key inflates indexes and, if used as the primary key, worsens the insert-locality overhead of Section 4.
- **Counter durability.** A per-device counter MUST survive reboot; otherwise the device must resync with the server or risk reissuing keys. Persisting every increment is expensive on constrained flash storage.

## 6.2 Options

- **Fixed-width encoding.** Hash or pack the composite into a fixed-width value and store it in a non-clustered unique index rather than clustering on it.
- **Block/batch allocation (HiLo).** Reserve a block of N counter values and persist only the high-watermark, bounding writes to once per N increments and tolerating a bounded gap on crash. This is the HiLo pattern used by ORMs such as NHibernate and EF.
- **Time-ordered IDs.** Snowflake, ULID, KSUID, and UUIDv7 combine a timestamp, a node/device identifier, and a small per-tick sequence. They avoid counter resync entirely, requiring only a reliable clock plus a small monotonic guard against clock regression.

## 6.3 Guidance

- If a monotonic counter is used, implementations SHOULD persist it in blocks (HiLo) rather than per-increment, and MUST ensure the persisted high-watermark is durable across reboot.
- Implementations MAY prefer time-ordered IDs (UUIDv7 / ULID / Snowflake) to avoid counter-resync complexity, provided the device clock is monotonic or guarded against regression.

---

# 7. Summary of Recommendations

| Concern | Preferred approach |
| --- | --- |
| SQL Server insert locality | Narrow surrogate clustered key + unique non-clustered index, or UUIDv7 |
| PostgreSQL insert locality | `uuid` primary key (acceptable); UUIDv7 preferred for high insert rates |
| Cross-engine portability (transition) | UUIDv7 (performs well on both) |
| Constrained-device entropy | Hardware TRNG / seeded CSPRNG, or device-scoped keys |
| Cross-device uniqueness | Device id + monotonic component |
| Counter durability | HiLo block allocation; durable high-watermark |
| Avoiding counter resync | Time-ordered IDs (UUIDv7 / ULID / Snowflake) |

---

# 8. References

- [RFC 9562 — Universally Unique IDentifiers (UUIDs), including v6/v7/v8][rfc9562]  
- [RFC 4122 — A Universally Unique IDentifier (UUID) URN Namespace (obsoleted by RFC 9562)][rfc4122]  
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
[pg-uuid]: https://www.postgresql.org/docs/current/datatype-uuid.html
[pg-cluster]: https://www.postgresql.org/docs/current/sql-cluster.html
[sqlskills-guids]: https://www.sqlskills.com/blogs/kimberly/guids-as-primary-keys-andor-the-clustering-key/
[comb-guids]: http://www.informit.com/articles/article.aspx?p=25862
[mining-ps-qs]: https://www.usenix.org/conference/usenixsecurity12/technical-sessions/presentation/heninger
[ulid]: https://github.com/ulid/spec
[snowflake]: https://github.com/twitter-archive/snowflake/tree/snowflake-2010
[ietf-idempotency-key]: https://datatracker.ietf.org/doc/draft-ietf-httpapi-idempotency-key-header/

---
