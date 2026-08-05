---
applyTo: "**/*.cs"
---
This document defines **mandatory architectural and domain requirements** for CSharp files.

These requirements are **authoritative** and **must be enforced during code review**.  
Any violation should be explicitly referenced by requirement ID (e.g. `R-3.1.28`) in review comments.

## R-1. Domain-Driven Design (DDD) Requirements

### R-1.1. Entities and Value Objects
- **R-1.1.1**: Entities must have a unique identifier (ID) that distinguishes them from other entities.
- **R-1.1.2**: Value Objects must be immutable and compared based on their attributes rather than identity.
- **R-1.1.3**: Domain models must have private setters and read-only collections.
- **R-1.1.4**: Domain models must only use static factory methods for creation.
- **R-1.1.6**: Domain models must not contain any data access or infrastructure logic.
- **R-1.1.7**: All changes to a domain aggregate must go through methods on the aggregate itself.
- **R-1.1.8**: Entities must validate their state and throw domain exceptions for invalid operations.
- **R-1.1.9**: Value Objects must be of the record type.
- **R-1.1.10**: Domain models must not depend on infrastructure types (e.g., EF Core attributes, JSON serialization attributes).

### R-1.2. Aggregates
- **R-1.2.1**: Each aggregate must have a single aggregate root that controls access to its members.
- **R-1.2.2**: External objects may only hold references to the aggregate root, not to internal entities.
- **R-1.2.3**: Aggregates must be designed around transactional consistency boundaries.
- **R-1.2.4**: Cross-aggregate references must use IDs, not direct object references.
- **R-1.2.5**: Only one aggregate may be modified per transaction.
- **R-1.2.6**: Aggregates must be kept small; prefer smaller aggregates over large ones.
- **R-1.2.7**: All writing to the database must be done through aggregate roots.

### R-1.3. Read models and queries
- **R-1.3.1**: All querying from the database must be done through read models. Domain models can be fetched only by their respective repositories for making changes.
- **R-1.3.2**: Read models must be of the type Record with a primary constructor and be immutable.
- **R-1.3.3**: Read models must be defined in the application layer.
- **R-1.3.4**: When querying data for read models, use projection to map directly to read models instead of loading full domain models.
- **R-1.3.5**: Read models must not contain navigation properties to domain models or other read models.

### R-1.4. Repositories
- **R-1.4.1**: Repositories must only exist for aggregate roots, not for child entities.
- **R-1.4.2**: Repository interfaces must be defined in the domain layer.
- **R-1.4.3**: Repository implementations must be defined in the infrastructure layer.
- **R-1.4.4**: Repositories must return domain models, not DTOs or database entities.
- **R-1.4.5**: Repositories must not expose `IQueryable` to prevent leaking query logic outside the repository.

### R-1.5. Application Services
- **R-1.5.1**: Application services must orchestrate use cases by coordinating domain models and repositories.
- **R-1.5.2**: Application services must not contain business logic; all business logic must reside in domain models.
- **R-1.5.3**: Application services must use repositories to load and save domain models.
- **R-1.5.4**: Application services must handle transactions and unit of work.
- **R-1.5.5**: Application services must only be used when orchestrating multiple domain models.

## R-2. Design requirements

### R-2.1. API Design
- **R-2.1.1**: Endpoints must receive either Request DTOs or primitive parameters.
- **R-2.1.2**: Endpoints must return Response DTOs.
- **R-2.1.3**: DTOs, Requests and Responses must be of the type Record with a primary constructor and be immutable.
- **R-2.1.4**: Endpoints must not expose domain models directly.
- **R-2.1.6**: Endpoints must not write data to the database or perform changes to domain models.

### R-2.2. Worker Service Design
- **R-2.2.1**: The Worker service must handle messaging through consumers.
- **R-2.2.2**: The Worker service must not contain business logic; all business logic must reside in domain models.
- **R-2.2.3**: Consumers must handle either Commands or Events.
- **R-2.2.4**: Consumers may work with domain models directly for simple use cases.
- **R-2.2.5**: Consumers must use application services to orchestrate complex use cases involving multiple domain models.
- **R-2.2.6**: Commands and Events must be of the type Record with a primary constructor and be immutable.
- **R-2.2.7**: Consumers must not pass commands or events to domain models or application services.

## R-3. Testing requirements

### R-3.1. Integration Tests
- **R-3.1.1**: All added code should be covered by integration tests.
- **R-3.1.2**: Integration tests must test the system end-to-end, including database but excluding external dependencies.
- **R-3.1.3**: Integration tests must use real database instances (e.g., Testcontainers) instead of in-memory databases.
- **R-3.1.4**: Integration tests must not mock domain models, repositories, or application services.
- **R-3.1.5**: Integration tests for endpoints must verify request and response DTOs.
- **R-3.1.6**: Integration tests for consumers must verify command and event handling.

## R-4. Code style
- **R-4.1.1**: Follow the official .NET coding conventions as per Microsoft documentation.
- **R-4.1.2**: Use meaningful names for classes, methods, and variables that reflect their purpose.
- **R-4.1.3**: Keep methods short and focused on a single responsibility.