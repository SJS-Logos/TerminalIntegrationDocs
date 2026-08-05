# AP-002 Implementation Guide: C++

**Version:** 0.1  
**Status:** Draft  
**Applies to:** AP-002, AP-004  

---

# 1. Introduction

This document demonstrates a concrete implementation of the service structure defined in AP-002 using C++.

It shows how to structure a service using the AP-002 Core model with `domain/`, `shared_kernel/`, `capabilities/`, and `application/`, and how Domain Services perform business logic while state is managed externally.

This example uses a simplified payment authorization service to illustrate the concepts.

> **📦 Compilable Example Available**  
> A fully compilable version of this example is available at:  
> [`examples/cpp/`](../../examples/cpp/)  
>
> You can build and run it with:
> ```bash
> cd examples/cpp
> ./build.sh           # Linux/macOS
> build.bat            # Windows
> 
> # Or manually:
> mkdir build && cd build
> cmake ..
> cmake --build . --config Release
> ./payment_cli help   # Linux/macOS
> .\Release\payment_cli.exe help  # Windows
> ```

**Key Architectural Decisions:**

- **Stateless Domain Services**: Business logic operates on Value Objects without entity classes
- **External State Management**: Identity and state persistence handled by the repository (database)
- **Shared Kernel**: Shared immutable business datatypes defined once in `shared_kernel/`, referenced across the Core
- **Template-Based Service Container**: Type-safe dependency injection using templates
- **Synchronous Request-Response**: Thread starts with an event (request), executes business logic, persists state, and ends with a message (response). No futures/async in domain/application layers; threads are short-lived.
- **Explicit Ownership**: Uses C++ Core Guidelines for ownership semantics:
  - `unique_ptr` for single ownership
  - Raw pointers for non-owning dependencies
  - **No `shared_ptr` unless actual shared ownership is required** (C++ Core Guidelines F.27)
- **Precise Money Handling**: Uses integer cents (int64_t) to avoid floating-point precision issues
- **No Double Exposure**: Money class provides formatted output, never exposes double

---

# 2. Project Structure

The example structure aligned to AP-002 is organized into the following libraries and executable:

**Location:** [`examples/cpp/`](../../examples/cpp/)

```
mypaymentservice_core/
├── domain/
│   ├── entities/
│   └── services/
├── shared_kernel/
│   ├── money/
│   ├── identifiers/
│   └── dates/
├── capabilities/
│   ├── persistence/
│   └── external_services/
└── application/
    ├── use_cases/
    ├── contracts/
    └── container/
```

See the [full README](../../examples/cpp/README.md) for complete project structure, build instructions, and usage examples.

**Build Configuration:** [`examples/cpp/CMakeLists.txt`](../../examples/cpp/CMakeLists.txt)

The Core library has no Host dependencies. Within the Core, `application/` may depend on `domain/`, `shared_kernel/`, and `capabilities/`, while `domain/` and `capabilities/` may both depend on `shared_kernel/`.

---

# 3. Core Layer

The Core comprises four peer folders — `domain/`, `shared_kernel/`, `capabilities/`, and `application/` — as defined in AP-002 §5.1. Shared Kernel and Capabilities are peers of the Domain, not part of it.

## 3.1 Shared Kernel

Shared immutable business datatypes belong in `shared_kernel/` and are reused across the Core.

### Money Value Object

**File:** `mypaymentservice_core/shared_kernel/money/money.h`

**Implementation:** `mypaymentservice_core/shared_kernel/money/money.cpp`

The `Money` value object:
- Stores amounts as integer cents (int64_t) to avoid floating-point precision issues
- Accepts string input for flexibility ("10.50" or "10,50")
- Provides `ToString()` for formatted display with thousands separators and currency
- **No double in public API** - enforces correct usage patterns
- Supports international formatting (US, European, Swiss formats)

**Key Design Decision:** Money uses integer cents internally and provides formatted string output. This eliminates floating-point precision errors and enforces proper display formatting with currency codes.

See: [Money Implementation Details](../../examples/cpp/MONEY_IMPLEMENTATION.md) and [No Double Policy](../../examples/cpp/NO_DOUBLE_POLICY.md)

### Payment Status

**File:** `mypaymentservice_core/shared_kernel/payment_status.h`

The `PaymentStatus` enum represents the authorization state (Pending, Authorized, Declined).

### Payment Record

**File:** `mypaymentservice_core/shared_kernel/payment_record.h`

The `PaymentRecord` struct represents a complete payment record with identity assigned by the repository.

## 3.2 Domain Services

Domain services contain business logic and work with Value Objects. They are stateless.

**File:** `mypaymentservice_core/domain/services/payment_authorization_service.h`

**Implementation:** `mypaymentservice_core/domain/services/payment_authorization_service.cpp`

The `PaymentAuthorizationService`:
- Stateless service that performs payment authorization business logic
- Works with Value Objects; state persistence is handled by repository
- Validates amount is positive
- Checks for fraud via `IFraudDetectionService` capability
- Validates amount against maximum limit
- Returns `PaymentAuthorizationResult` with decision and optional decline reason

## 3.3 Capabilities (Domain Abstractions)

Capabilities are domain-owned abstractions that describe what the Domain requires.

### Fraud Detection Capability

**File:** `mypaymentservice_core/capabilities/external_services/fraud_detection_service.h`

The `IFraudDetectionService` interface:
- Abstract interface owned by the Domain
- Accepts amount as int64_t cents (not double) for precision
- Implementation provided by adapters layer

### Payment Repository Capability

**File:** `mypaymentservice_core/capabilities/persistence/payment_repository.h`

The `IPaymentRepository` interface:
- Defines persistence operations in domain terms
- Responsible for identity generation and state management
- Returns `std::optional<PaymentRecord>` for nullable results

---

# 4. Application Layer

## 4.1 Contract Models

Contract models reference shared business datatypes from `shared_kernel/`.

**Authorize Payment Request:** `mypaymentservice_core/application/contracts/authorize_payment_request.h`

**Authorize Payment Response:** `mypaymentservice_core/application/contracts/authorize_payment_response.h`

**Get Payment Response:** `mypaymentservice_core/application/contracts/get_payment_response.h`

Contract models reference `Money` and other Value Objects directly from the Domain - no duplication.

## 4.2 Service Container (Dependency Injection)

**File:** `mypaymentservice_core/application/container/service_container.h`

A simple template-based service container that:
- Owns service instances with `unique_ptr`
- Provides type-safe resolution
- Manages service lifetimes
- No external dependencies required

## 4.3 Use Cases

Use cases receive their dependencies through constructor injection.

### Authorize Payment Use Case

**File:** `mypaymentservice_core/application/use_cases/authorize_payment_use_case.h`

**Implementation:** `mypaymentservice_core/application/use_cases/authorize_payment_use_case.cpp`

The `AuthorizePaymentUseCase`:
- Orchestrates payment authorization workflow
- Executes business logic via `PaymentAuthorizationService`
- Persists state via `IPaymentRepository`
- Returns contract model with result

### Get Payment Use Case

**File:** `mypaymentservice_core/application/use_cases/get_payment_use_case.h`

**Implementation:** `mypaymentservice_core/application/use_cases/get_payment_use_case.cpp`

Simple retrieval use case that queries the repository.

---

# 5. Composition Root

Configuration is loaded and services are registered in a container.

**File:** `mypaymentservice_cli_host/main.cpp`

The composition root:
- Creates the service container
- Registers adapters (implementations of domain capabilities)
- Registers domain services with configuration
- Creates use cases with resolved dependencies
- Routes to command handlers

---

# 6. Adapter Registration

Adapters implement domain capabilities.

The key point for AP-002 is:
- `capabilities/` defines **what** the Core needs
- `shared_kernel/` holds shared business datatypes
- host and infrastructure examples are covered by later APs

---

# 7. Key Points

1. **The Core has no Host dependencies**: `mypaymentservice_core` links against no Host targets.

2. **`application/` depends inward**: it references only `domain/`, `shared_kernel/`, and `capabilities/`.

3. **Shared business datatypes are centralized**: `Money`, `PaymentStatus`, and `PaymentRecord` are defined once in `shared_kernel/` and reused across the Core.

4. **Domain Services are stateless**: `PaymentAuthorizationService` performs business logic on Value Objects without managing state.

5. **State is external**: The repository (database) manages identity generation, persistence, and retrieval. The domain doesn't need entity classes for simple operations.

6. **Template-based service container**: Provides type-safe dependency injection and manages service lifetimes without requiring a framework.

7. **Business logic is pure**: Authorization logic can be tested without any database or infrastructure.

8. **Configuration in composition root**: Domain services accept configuration as constructor parameters rather than hardcoding values, allowing flexibility and testability.

9. **Explicit ownership semantics** (C++ Core Guidelines):
   - **Service container owns services**: Uses `unique_ptr` for single ownership
   - **Dependencies are non-owning**: Services take raw pointers (`T*`) to dependencies
   - **No `shared_ptr` by default**: Only use when actual shared ownership is needed (F.27)
   - **Rationale**: Clear ownership prevents memory leaks, clarifies lifetimes, and avoids the common anti-pattern of `shared_ptr` everywhere

10. **Precise money handling**: Money uses integer cents internally, eliminating floating-point precision issues. No double in public API.

---

# 8. Testing

Business logic can be tested in complete isolation using GoogleMock.

Example test structure:

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "domain/services/payment_authorization_service.h"

using namespace mypaymentservice::core::domain;

class MockFraudDetectionService : public abstractions::IFraudDetectionService {
public:
    MOCK_METHOD(bool, IsFraudulent,
        (int64_t, const std::string&, const std::string&), (override));
};

TEST(PaymentAuthorizationServiceTest, Authorize_Declines_When_Amount_Exceeds_Maximum) {
    MockFraudDetectionService fraud_detection;
    EXPECT_CALL(fraud_detection, IsFraudulent(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));

    Money maximum_amount = Money::FromCents(1000000, "USD");  // $10,000.00
    services::PaymentAuthorizationService service(&fraud_detection, maximum_amount);
    Money amount = Money::FromCents(1500000, "USD");  // $15,000.00

    auto result = service.Authorize(amount, "MERCH-001");

    ASSERT_FALSE(result.is_authorized);
    ASSERT_EQ(result.status, shared_kernel::PaymentStatus::Declined);
    ASSERT_TRUE(result.decline_reason.has_value());
}
```

---

# 9. Running the Example

See [QUICKSTART.md](../../examples/cpp/QUICKSTART.md) for detailed build and run instructions.

Quick example:
```bash
cd examples/cpp/build
./payment_cli authorize --amount 100.00 --currency USD --merchant MERCH-001
# Output: Payment ID: PAY-000001, Authorized: YES, Amount: 100.00 USD

./payment_cli authorize --amount 5000.01 --currency USD --merchant MERCH-001
# Output: Declined - Suspected fraud (amount > $5,000.00)
```

---

**End of Document**
