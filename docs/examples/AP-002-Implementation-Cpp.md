# AP-002 Implementation Guide: C++

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-002, AP-004  

---

# 1. Introduction

This document demonstrates a concrete implementation of the service structure defined in AP-002 using C++.

It shows how to structure a service using the Domain and Application units, how to define shared Value Objects (per AP-004 §7), and how Domain Services perform business logic while state is managed externally.

This example uses a simplified payment authorization service to illustrate the concepts.

**Key Architectural Decisions:**

- **Stateless Domain Services**: Business logic operates on Value Objects without entity classes
- **External State Management**: Identity and state persistence handled by the repository (database)
- **Shared Value Objects**: Value Objects defined once in Domain, referenced by Application contracts (AP-004 §7)
- **Template-Based Service Container**: Type-safe dependency injection using templates
- **Synchronous Request-Response**: Thread starts with an event (request), executes business logic, persists state, and ends with a message (response). No futures/async in domain/application layers; threads are short-lived.
- **Explicit Ownership**: Uses C++ Core Guidelines for ownership semantics:
  - `unique_ptr` for single ownership
  - Raw pointers for non-owning dependencies
  - **No `shared_ptr` unless actual shared ownership is required** (C++ Core Guidelines F.27)

---

# 2. Project Structure

A C++ service following AP-002 consists of the following library targets:

```
company_payment_service/
├── company_payment_service_domain/
│   ├── value_objects/         (Shared with contracts per AP-004 §7)
│   ├── services/
│   └── abstractions/          (Capabilities)
└── company_payment_service_application/
    ├── use_cases/
    ├── contracts/             (References Domain ValueObjects)
    └── container/             (Service container for DI)
```

**CMake Example**:

```cmake
# Domain library
add_library(company_payment_service_domain STATIC
    domain/value_objects/money.cpp
    domain/value_objects/payment_record.cpp
    domain/services/payment_authorization_service.cpp
)
target_include_directories(company_payment_service_domain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Application library
add_library(company_payment_service_application STATIC
    application/use_cases/authorize_payment_use_case.cpp
    application/use_cases/get_payment_use_case.cpp
    application/container/service_container.cpp
)
target_link_libraries(company_payment_service_application PUBLIC
    company_payment_service_domain
)
```

---

# 3. Domain Layer

## 3.1 Value Objects

Value Objects are immutable domain concepts shared between Domain and Application (AP-004 §7).

```cpp
// company_payment_service_domain/value_objects/money.h
#pragma once

#include <string>
#include <stdexcept>

namespace company::payment_service::domain::value_objects {

/// Shared Value Object representing monetary amounts.
/// Used by both Domain logic and Application contracts (AP-004 §7).
class Money {
public:
    Money(double amount, const std::string& currency);

    static Money Zero(const std::string& currency);

    Money Add(const Money& other) const;
    bool IsPositive() const;
    bool IsGreaterThan(const Money& other) const;

    double GetAmount() const { return amount_; }
    const std::string& GetCurrency() const { return currency_; }

    bool operator==(const Money& other) const;
    bool operator!=(const Money& other) const;

private:
    double amount_;
    std::string currency_;
};

} // namespace company::payment_service::domain::value_objects
```

<details>
<summary><b>Implementation</b>: <code>money.cpp</code></summary>

```cpp
// company_payment_service_domain/value_objects/money.cpp
#include "value_objects/money.h"

namespace company::payment_service::domain::value_objects {

Money::Money(double amount, const std::string& currency)
    : amount_(amount), currency_(currency) {
}

Money Money::Zero(const std::string& currency) {
    return Money(0.0, currency);
}

Money Money::Add(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot add money with different currencies");
    }
    return Money(amount_ + other.amount_, currency_);
}

bool Money::IsPositive() const {
    return amount_ > 0;
}

bool Money::IsGreaterThan(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot compare money with different currencies");
    }
    return amount_ > other.amount_;
}

bool Money::operator==(const Money& other) const {
    return amount_ == other.amount_ && currency_ == other.currency_;
}

bool Money::operator!=(const Money& other) const {
    return !(*this == other);
}

} // namespace company::payment_service::domain::value_objects
```

</details>

```cpp
// company_payment_service_domain/value_objects/payment_status.h
#pragma once

namespace company::payment_service::domain::value_objects {

/// Shared Value Object representing payment authorization status.
enum class PaymentStatus {
    Pending,
    Authorized,
    Declined
};

} // namespace company::payment_service::domain::value_objects
```

```cpp
// company_payment_service_domain/value_objects/payment_record.h
#pragma once

#include "money.h"
#include "payment_status.h"
#include <string>
#include <chrono>
#include <optional>

namespace company::payment_service::domain::value_objects {

/// Value Object representing a payment record.
/// The identity (id) is assigned by the repository/database.
struct PaymentRecord {
    std::string id;
    Money amount;
    std::string merchant_id;
    PaymentStatus status;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::string> decline_reason;
};

} // namespace company::payment_service::domain::value_objects
```

## 3.2 Domain Services

Domain services contain business logic and work with Value Objects. They are stateless.

```cpp
// company_payment_service_domain/services/payment_authorization_service.h
#pragma once

#include "value_objects/money.h"
#include "value_objects/payment_status.h"
#include "abstractions/fraud_detection_service.h"
#include <memory>
#include <optional>
#include <string>

namespace company::payment_service::domain::services {

/// Result of payment authorization business logic
struct PaymentAuthorizationResult {
    bool is_authorized;
    value_objects::PaymentStatus status;
    std::optional<std::string> decline_reason;
};

/// Stateless service that performs payment authorization business logic.
/// Works with Value Objects; state persistence is handled by repository.
class PaymentAuthorizationService {
public:
    explicit PaymentAuthorizationService(
        abstractions::IFraudDetectionService* fraud_detection,
        const value_objects::Money& maximum_amount);

    PaymentAuthorizationResult Authorize(
        const value_objects::Money& amount,
        const std::string& merchant_id);

private:
    abstractions::IFraudDetectionService* fraud_detection_;  // Non-owning
    value_objects::Money maximum_amount_;
};

} // namespace company::payment_service::domain::services
```

```cpp
// company_payment_service_domain/services/payment_authorization_service.cpp
#include "services/payment_authorization_service.h"

namespace company::payment_service::domain::services {

PaymentAuthorizationService::PaymentAuthorizationService(
    abstractions::IFraudDetectionService* fraud_detection,
    const value_objects::Money& maximum_amount)
    : fraud_detection_(fraud_detection)
    , maximum_amount_(maximum_amount) {
}

PaymentAuthorizationResult PaymentAuthorizationService::Authorize(
    const value_objects::Money& amount,
    const std::string& merchant_id) {

    // Business rule: Amount must be positive
    if (!amount.IsPositive()) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Amount must be positive"
        };
    }

    // Business rule: Check for fraud
    bool is_fraudulent = fraud_detection_->IsFraudulent(
        amount.GetAmount(),
        amount.GetCurrency(),
        merchant_id);

    if (is_fraudulent) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Suspected fraud"
        };
    }

    // Business rule: Amount limits
    if (amount.GetCurrency() == maximum_amount_.GetCurrency() &&
        amount.IsGreaterThan(maximum_amount_)) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Amount exceeds maximum limit"
        };
    }

    return PaymentAuthorizationResult{
        true,
        value_objects::PaymentStatus::Authorized,
        std::nullopt
    };
}

} // namespace company::payment_service::domain::services
```

## 3.3 Capabilities (Domain Abstractions)

Capabilities are domain-owned abstractions that describe what the Domain requires.

```cpp
// company_payment_service_domain/abstractions/fraud_detection_service.h
#pragma once

#include <string>

namespace company::payment_service::domain::abstractions {

/// Capability: Fraud detection expressed in domain terms.
class IFraudDetectionService {
public:
    virtual ~IFraudDetectionService() = default;

    virtual bool IsFraudulent(
        double amount,
        const std::string& currency,
        const std::string& merchant_id) = 0;
};

} // namespace company::payment_service::domain::abstractions
```

```cpp
// company_payment_service_domain/abstractions/payment_repository.h
#pragma once

#include "value_objects/payment_record.h"
#include "value_objects/money.h"
#include "value_objects/payment_status.h"
#include <memory>
#include <optional>
#include <string>

namespace company::payment_service::domain::abstractions {

/// Capability: Persistence expressed in domain terms.
/// Responsible for identity generation and state management.
class IPaymentRepository {
public:
    virtual ~IPaymentRepository() = default;

    virtual std::optional<value_objects::PaymentRecord> GetById(
        const std::string& id) = 0;

    /// Saves a payment record. The repository assigns the id.
    virtual value_objects::PaymentRecord Save(
        const value_objects::Money& amount,
        const std::string& merchant_id,
        value_objects::PaymentStatus status,
        const std::optional<std::string>& decline_reason) = 0;
};

} // namespace company::payment_service::domain::abstractions
```

---

# 4. Application Layer

## 4.1 Contract Models

Contract models reference shared Value Objects from the Domain (AP-004 §7).

```cpp
// company_payment_service_application/contracts/authorize_payment_request.h
#pragma once

#include "domain/value_objects/money.h"
#include <string>

namespace company::payment_service::application::contracts {

/// Contract model that references shared Value Objects.
/// No duplication - Money is defined once in Domain.
struct AuthorizePaymentRequest {
    domain::value_objects::Money amount;
    std::string merchant_id;
};

} // namespace company::payment_service::application::contracts
```

```cpp
// company_payment_service_application/contracts/authorize_payment_response.h
#pragma once

#include "domain/value_objects/money.h"
#include "domain/value_objects/payment_status.h"
#include <string>
#include <optional>

namespace company::payment_service::application::contracts {

/// Contract model that references shared Value Objects.
struct AuthorizePaymentResponse {
    std::string payment_id;
    bool is_authorized;
    domain::value_objects::PaymentStatus status;
    domain::value_objects::Money amount;
    std::optional<std::string> decline_reason;
};

} // namespace company::payment_service::application::contracts
```

```cpp
// company_payment_service_application/contracts/get_payment_response.h
#pragma once

#include "domain/value_objects/payment_record.h"

namespace company::payment_service::application::contracts {

/// Contract model for retrieving payment records.
using GetPaymentResponse = domain::value_objects::PaymentRecord;

} // namespace company::payment_service::application::contracts
```

## 4.2 Service Container (Dependency Injection)

A simple service container manages dependency lifetimes and provides type-safe access.

```cpp
// company_payment_service_application/container/service_container.h
#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>

namespace company::payment_service::application::container {

/// Simple service container for dependency injection.
/// Owns service instances and provides type-safe access.
class ServiceContainer {
public:
    /// Register a service instance
    template<typename TInterface>
    void Register(std::unique_ptr<TInterface> service) {
        auto type = std::type_index(typeid(TInterface));
        services_[type] = std::move(service);
    }

    /// Resolve a service by interface type
    template<typename TInterface>
    TInterface* Resolve() {
        auto type = std::type_index(typeid(TInterface));
        auto it = services_.find(type);
        if (it == services_.end()) {
            throw std::runtime_error("Service not registered");
        }
        return static_cast<TInterface*>(it->second.get());
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<void, void(*)(void*)>> services_;

    // Custom deleter that maintains type information
    template<typename T>
    static void TypedDeleter(void* ptr) {
        delete static_cast<T*>(ptr);
    }

    // Specialization for storing typed unique_ptrs
    template<typename T>
    std::unique_ptr<void, void(*)(void*)> MakeErased(std::unique_ptr<T> ptr) {
        return std::unique_ptr<void, void(*)(void*)>(ptr.release(), &TypedDeleter<T>);
    }
};

// Simplified version using any
class ServiceContainer {
public:
    template<typename TInterface>
    void Register(std::unique_ptr<TInterface> service) {
        services_[std::type_index(typeid(TInterface))] = std::move(service);
    }

    template<typename TInterface>
    TInterface* Resolve() {
        auto type = std::type_index(typeid(TInterface));
        auto it = services_.find(type);
        if (it == services_.end()) {
            throw std::runtime_error("Service not registered");
        }
        return static_cast<TInterface*>(it->second.get());
    }

private:
    struct ServiceHolder {
        virtual ~ServiceHolder() = default;
    };

    template<typename T>
    struct TypedHolder : ServiceHolder {
        std::unique_ptr<T> instance;
        explicit TypedHolder(std::unique_ptr<T> inst) : instance(std::move(inst)) {}
    };

    std::unordered_map<std::type_index, std::unique_ptr<ServiceHolder>> services_;
};

} // namespace company::payment_service::application::container
```

**Simplified implementation**: [service_container.cpp](code-snippets/service_container.cpp)

## 4.3 Use Cases

Use cases receive their dependencies through constructor injection.

```cpp
// company_payment_service_application/use_cases/authorize_payment_use_case.h
#pragma once

#include "contracts/authorize_payment_request.h"
#include "contracts/authorize_payment_response.h"
#include "domain/services/payment_authorization_service.h"
#include "domain/abstractions/payment_repository.h"

namespace company::payment_service::application::use_cases {

/// Use case that orchestrates payment authorization.
/// Domain service performs business logic.
/// Repository manages state and identity.
class AuthorizePaymentUseCase {
public:
    AuthorizePaymentUseCase(
        domain::services::PaymentAuthorizationService* auth_service,
        domain::abstractions::IPaymentRepository* repository);

    contracts::AuthorizePaymentResponse Execute(
        const contracts::AuthorizePaymentRequest& request);

private:
    domain::services::PaymentAuthorizationService* auth_service_;  // Non-owning
    domain::abstractions::IPaymentRepository* repository_;         // Non-owning
};

} // namespace company::payment_service::application::use_cases
```

```cpp
// company_payment_service_application/use_cases/authorize_payment_use_case.cpp
#include "use_cases/authorize_payment_use_case.h"

namespace company::payment_service::application::use_cases {

AuthorizePaymentUseCase::AuthorizePaymentUseCase(
    domain::services::PaymentAuthorizationService* auth_service,
    domain::abstractions::IPaymentRepository* repository)
    : auth_service_(auth_service)
    , repository_(repository) {
}

contracts::AuthorizePaymentResponse 
AuthorizePaymentUseCase::Execute(
    const contracts::AuthorizePaymentRequest& request) {

    // Execute business logic through stateless domain service
    auto result = auth_service_->Authorize(request.amount, request.merchant_id);

    // Persist state through repository (repository assigns id)
    auto saved_payment = repository_->Save(
        request.amount,
        request.merchant_id,
        result.status,
        result.decline_reason);

    // Return contract model
    return contracts::AuthorizePaymentResponse{
        saved_payment.id,
        result.is_authorized,
        saved_payment.status,
        saved_payment.amount,
        saved_payment.decline_reason
    };
}

} // namespace company::payment_service::application::use_cases
```

```cpp
// company_payment_service_application/use_cases/get_payment_use_case.h
#pragma once

#include "contracts/get_payment_response.h"
#include "domain/abstractions/payment_repository.h"
#include <optional>
#include <string>

namespace company::payment_service::application::use_cases {

/// Use case for retrieving payment records.
class GetPaymentUseCase {
public:
    explicit GetPaymentUseCase(domain::abstractions::IPaymentRepository* repository);

    std::optional<contracts::GetPaymentResponse> Execute(
        const std::string& payment_id);

private:
    domain::abstractions::IPaymentRepository* repository_;  // Non-owning
};

} // namespace company::payment_service::application::use_cases
```

```cpp
// company_payment_service_application/use_cases/get_payment_use_case.cpp
#include "use_cases/get_payment_use_case.h"

namespace company::payment_service::application::use_cases {

GetPaymentUseCase::GetPaymentUseCase(domain::abstractions::IPaymentRepository* repository)
    : repository_(repository) {
}

std::optional<contracts::GetPaymentResponse> 
GetPaymentUseCase::Execute(const std::string& payment_id) {
    return repository_->GetById(payment_id);
}
}

} // namespace company::payment_service::application::use_cases
```

---

# 5. Composition Root

Configuration is loaded and services are registered in a container.

```cpp
// main.cpp or composition root
#include "application/container/service_container.h"
#include "application/use_cases/authorize_payment_use_case.h"
#include "application/use_cases/get_payment_use_case.h"
#include "domain/services/payment_authorization_service.h"
#include "adapters/sqlite_payment_repository.h"
#include "adapters/third_party_fraud_detection_service.h"
#include "config/configuration.h"

int main() {
    // Load configuration
    auto config = Configuration::Load("appsettings.json");

    // Create service container
    company::payment_service::application::container::ServiceContainer container;

    // Register domain configuration
    auto maximum_amount = company::payment_service::domain::value_objects::Money(
        config.GetDouble("Payment:MaximumAmount"),
        config.GetString("Payment:Currency"));

    // Register capability implementations (container takes ownership)
    container.Register<domain::abstractions::IPaymentRepository>(
        std::make_unique<adapters::SqlitePaymentRepository>(config));

    container.Register<domain::abstractions::IFraudDetectionService>(
        std::make_unique<adapters::ThirdPartyFraudDetectionService>(config));

    // Register domain services
    container.Register<domain::services::PaymentAuthorizationService>(
        std::make_unique<domain::services::PaymentAuthorizationService>(
            container.Resolve<domain::abstractions::IFraudDetectionService>(),
            maximum_amount));

    // Create and use use case with resolved dependencies
    company::payment_service::application::use_cases::AuthorizePaymentUseCase 
        use_case(
            container.Resolve<domain::services::PaymentAuthorizationService>(),
            container.Resolve<domain::abstractions::IPaymentRepository>());

    auto request = company::payment_service::application::contracts::
        AuthorizePaymentRequest{
            company::payment_service::domain::value_objects::Money(100.0, "USD"),
            "MERCH-001"
        };

    auto response = use_case.Execute(request);

    return 0;
}
```

---

# 6. Adapter Registration

Adapters implement domain capabilities. Their concrete implementations are outside the scope of AP-002 and will be covered in later APs on infrastructure.

The key point for AP-002 is:
- Domain defines **what** it needs (capabilities/abstractions)
- Adapters provide **how** it's implemented (covered in later APs)
- Service container connects them

---

# 7. Key Points

1. **The Domain has no external dependencies**: `company_payment_service_domain` links against no other service targets.

2. **The Application depends only on the Domain**: `company_payment_service_application` links only against `company_payment_service_domain`.

3. **Value Objects are shared** (AP-004 §7): `Money`, `PaymentStatus`, and `PaymentRecord` are defined once in the Domain and referenced by Application contracts. No duplication.

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

---

# 8. Testing

Business logic can be tested in complete isolation using GoogleMock.

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "domain/services/payment_authorization_service.h"

using namespace company::payment_service::domain;

class MockFraudDetectionService : public abstractions::IFraudDetectionService {
public:
    MOCK_METHOD(bool, IsFraudulent,
        (double, const std::string&, const std::string&), (override));
};

TEST(PaymentAuthorizationServiceTest, Authorize_Declines_When_Amount_Exceeds_Maximum) {
    MockFraudDetectionService fraud_detection;
    EXPECT_CALL(fraud_detection, IsFraudulent(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));

    value_objects::Money maximum_amount(10000.0, "USD");
    services::PaymentAuthorizationService service(&fraud_detection, maximum_amount);
    value_objects::Money amount(15000.0, "USD");  // Exceeds limit

    auto result = service.Authorize(amount, "MERCH-001");

    ASSERT_FALSE(result.is_authorized);
    ASSERT_EQ(result.status, value_objects::PaymentStatus::Declined);
    ASSERT_TRUE(result.decline_reason.has_value());
    ASSERT_NE(result.decline_reason->find("exceeds maximum"), std::string::npos);
}
```

---

**End of Document**
