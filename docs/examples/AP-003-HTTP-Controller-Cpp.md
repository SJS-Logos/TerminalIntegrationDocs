# HTTP Controller Example: C++

# HTTP Controller Example: C++

**Version:** 1.0  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C++](AP-002-Implementation-Cpp.md)

---

## 1. Introduction

This example demonstrates how to add an HTTP controller using Qt's HTTP server that receives requests and invokes use cases from the [C++ implementation example](AP-002-Implementation-Cpp.md).

**Key Points:**
- Qt HTTP server receives requests
- Maps HTTP payload to Contract Model (Request)
- Invokes Use Case synchronously
- Maps result to HTTP response (JSON)
- Thread lifecycle: Request ? Execute ? Response ? End

**Documentation Approach:**
- Domain and Application have Doxygen comments
- HTTP layer references domain headers
- Generated documentation links HTTP endpoints to domain concepts

---

## 2. Project Structure

```
logos_payment_core/                     (From AP-002 example)
├── domain/
├── shared_kernel/
├── capabilities/
└── application/
logos_payment_http_host/                (HTTP incoming)
├── controllers/
│   ├── payments_controller.h
│   ├── payments_controller.cpp
├── mappings/
│   ├── payment_dto.h
│   ├── payment_dto.cpp
├── configuration/
│   ├── http_server.h
│   └── http_server.cpp
└── main.cpp
```

---

## 3. HTTP DTOs (Transport Models)

### 3.1 Payment DTOs

```cpp
// logos_payment_http_host/mappings/payment_dto.h
#pragma once

#include <QJsonObject>
#include <QString>
#include <QDateTime>
#include <optional>

namespace logos::payment::http_host::mappings {

/// @brief HTTP request for payment authorization
/// @details Maps to application::contracts::AuthorizePaymentRequest
/// @see application::contracts::AuthorizePaymentRequest
/// @see domain::value_objects::Money
struct AuthorizePaymentDto {
    double amount;
    QString currency;
    QString merchant_id;

    /// Parse from JSON
    static std::optional<AuthorizePaymentDto> FromJson(const QJsonObject& json);

    /// Convert to JSON
    QJsonObject ToJson() const;
};

/// @brief HTTP response for payment authorization
/// @details Maps to application::contracts::AuthorizePaymentResponse
/// @see application::contracts::AuthorizePaymentResponse
/// @see domain::value_objects::PaymentStatus
struct PaymentAuthorizationDto {
    QString payment_id;
    bool is_authorized;
    QString status;
    double amount;
    QString currency;
    std::optional<QString> decline_reason;

    /// Convert to JSON
    QJsonObject ToJson() const;
};

/// @brief HTTP response for payment details
/// @details Maps to application::contracts::GetPaymentResponse
/// @see application::contracts::GetPaymentResponse
struct PaymentDetailsDto {
    QString payment_id;
    double amount;
    QString currency;
    QString merchant_id;
    QString status;
    QDateTime created_at;
    std::optional<QString> decline_reason;

    /// Convert to JSON
    QJsonObject ToJson() const;
};

} // namespace logos::payment_service::http::models
```

### 3.2 DTO Implementation

```cpp
// logos_payment_http_host/mappings/payment_dto.cpp
#include "payment_dto.h"
#include <QJsonValue>

namespace logos::payment::http_host::mappings {

std::optional<AuthorizePaymentDto> AuthorizePaymentDto::FromJson(const QJsonObject& json) {
    if (!json.contains("amount") || !json.contains("currency") || !json.contains("merchantId")) {
        return std::nullopt;
    }

    AuthorizePaymentDto dto;
    dto.amount = json["amount"].toDouble();
    dto.currency = json["currency"].toString();
    dto.merchant_id = json["merchantId"].toString();

    return dto;
}

QJsonObject AuthorizePaymentDto::ToJson() const {
    QJsonObject json;
    json["amount"] = amount;
    json["currency"] = currency;
    json["merchantId"] = merchant_id;
    return json;
}

QJsonObject PaymentAuthorizationDto::ToJson() const {
    QJsonObject json;
    json["paymentId"] = payment_id;
    json["isAuthorized"] = is_authorized;
    json["status"] = status;
    json["amount"] = amount;
    json["currency"] = currency;

    if (decline_reason) {
        json["declineReason"] = *decline_reason;
    }

    return json;
}

QJsonObject PaymentDetailsDto::ToJson() const {
    QJsonObject json;
    json["paymentId"] = payment_id;
    json["amount"] = amount;
    json["currency"] = currency;
    json["merchantId"] = merchant_id;
    json["status"] = status;
    json["createdAt"] = created_at.toString(Qt::ISODate);

    if (decline_reason) {
        json["declineReason"] = *decline_reason;
    }

    return json;
}

} // namespace logos::payment_service::http::models
```

---

## 4. Payment Controller

### 4.1 Controller Header

```cpp
// logos_payment_http_host/controllers/payments_controller.h
#pragma once

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include "application/container/service_container.h"
#include "application/use_cases/authorize_payment_use_case.h"
#include "application/use_cases/get_payment_use_case.h"

namespace logos::payment::http_host::controllers {

/// @brief HTTP controller for payment operations
/// @details Handles HTTP requests for payment authorization and retrieval.
///          Maps HTTP DTOs to domain contracts and invokes use cases.
/// 
/// Thread lifecycle: Request ? Map DTO ? Invoke Use Case ? Map Result ? Response ? End
/// 
/// @note This controller contains NO business logic. All business decisions
///       are made in the domain layer.
/// 
/// @see application::use_cases::AuthorizePaymentUseCase
/// @see application::use_cases::GetPaymentUseCase
class PaymentsController {
public:
    explicit PaymentsController(application::container::ServiceContainer* container);

    /// @brief POST /api/payments/authorize
    /// @details Authorizes a payment request
    /// @param request HTTP request containing payment details (JSON)
    /// @return HTTP 200 with authorization result, or 400 on invalid request
    /// 
    /// Request JSON:
    /// @code{.json}
    /// {
    ///   "amount": 100.00,
    ///   "currency": "USD",
    ///   "merchantId": "MERCH-001"
    /// }
    /// @endcode
    /// 
    /// Response JSON:
    /// @code{.json}
    /// {
    ///   "paymentId": "uuid",
    ///   "isAuthorized": true,
    ///   "status": "Authorized",
    ///   "amount": 100.00,
    ///   "currency": "USD"
    /// }
    /// @endcode
    /// 
    /// @see models::AuthorizePaymentDto
    /// @see application::contracts::AuthorizePaymentRequest
    QHttpServerResponse AuthorizePayment(const QHttpServerRequest& request);

    /// @brief GET /api/payments/{id}
    /// @details Retrieves payment details by ID
    /// @param payment_id Payment identifier
    /// @return HTTP 200 with payment details, or 404 if not found
    /// 
    /// @see models::PaymentDetailsDto
    /// @see application::contracts::GetPaymentResponse
    QHttpServerResponse GetPayment(const QString& payment_id);

private:
    application::use_cases::AuthorizePaymentUseCase* authorize_use_case_;  // Non-owning
    application::use_cases::GetPaymentUseCase* get_use_case_;              // Non-owning
};

} // namespace logos::payment_service::http::controllers
```

### 4.2 Controller Implementation

```cpp
// logos_payment_service_http/controllers/payments_controller.cpp
#include "payments_controller.h"
#include "models/payment_dto.h"
#include "domain/value_objects/money.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace logos::payment_service::http::controllers {

PaymentsController::PaymentsController(application::container::ServiceContainer* container) {
    // Resolve use cases from container
    authorize_use_case_ = container->Resolve<application::use_cases::AuthorizePaymentUseCase>();
    get_use_case_ = container->Resolve<application::use_cases::GetPaymentUseCase>();
}

QHttpServerResponse PaymentsController::AuthorizePayment(const QHttpServerRequest& request) {
    // Parse JSON body
    QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (!doc.isObject()) {
        return QHttpServerResponse("application/json", 
                                   QJsonDocument(QJsonObject{{"error", "Invalid JSON"}}).toJson(),
                                   QHttpServerResponse::StatusCode::BadRequest);
    }

    // Map HTTP DTO from JSON
    auto dto_opt = models::AuthorizePaymentDto::FromJson(doc.object());
    if (!dto_opt) {
        return QHttpServerResponse("application/json",
                                   QJsonDocument(QJsonObject{{"error", "Missing required fields"}}).toJson(),
                                   QHttpServerResponse::StatusCode::BadRequest);
    }
    auto dto = *dto_opt;

    // Map HTTP DTO ? Contract Model (using shared Value Objects)
    application::contracts::AuthorizePaymentRequest use_case_request{
        domain::value_objects::Money(dto.amount, dto.currency.toStdString()),
        dto.merchant_id.toStdString()
    };

    // Execute use case (synchronous, short-lived thread)
    auto result = authorize_use_case_->Execute(use_case_request);

    // Map result ? HTTP DTO
    models::PaymentAuthorizationDto response_dto;
    response_dto.payment_id = QString::fromStdString(result.id);
    response_dto.is_authorized = result.is_authorized;
    response_dto.status = QString::fromStdString(
        PaymentStatusToString(result.status));
    response_dto.amount = result.amount.GetAmount();
    response_dto.currency = QString::fromStdString(result.amount.GetCurrency());

    if (result.decline_reason) {
        response_dto.decline_reason = QString::fromStdString(*result.decline_reason);
    }

    // Return JSON response
    QJsonDocument response_doc(response_dto.ToJson());
    return QHttpServerResponse("application/json", 
                               response_doc.toJson(),
                               QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse PaymentsController::GetPayment(const QString& payment_id) {
    // Execute use case
    auto result = get_use_case_->Execute(payment_id.toStdString());

    if (!result) {
        return QHttpServerResponse("application/json",
                                   QJsonDocument(QJsonObject{{"error", "Payment not found"}}).toJson(),
                                   QHttpServerResponse::StatusCode::NotFound);
    }

    // Map result ? HTTP DTO
    models::PaymentDetailsDto response_dto;
    response_dto.payment_id = QString::fromStdString(result->id);
    response_dto.amount = result->amount.GetAmount();
    response_dto.currency = QString::fromStdString(result->amount.GetCurrency());
    response_dto.merchant_id = QString::fromStdString(result->merchant_id);
    response_dto.status = QString::fromStdString(PaymentStatusToString(result->status));
    response_dto.created_at = QDateTime::fromSecsSinceEpoch(
        std::chrono::system_clock::to_time_t(result->created_at));

    if (result->decline_reason) {
        response_dto.decline_reason = QString::fromStdString(*result->decline_reason);
    }

    // Return JSON response
    QJsonDocument response_doc(response_dto.ToJson());
    return QHttpServerResponse("application/json",
                               response_doc.toJson(),
                               QHttpServerResponse::StatusCode::Ok);
}

// Helper to convert PaymentStatus enum to string
std::string PaymentsController::PaymentStatusToString(
    domain::value_objects::PaymentStatus status) {
    switch (status) {
        case domain::value_objects::PaymentStatus::Pending:
            return "Pending";
        case domain::value_objects::PaymentStatus::Authorized:
            return "Authorized";
        case domain::value_objects::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment_service::http::controllers
```

---

## 5. HTTP Server Setup

### 5.1 HTTP Server

```cpp
// logos_payment_service_http/http_server.h
#pragma once

#include <QHttpServer>
#include "application/container/service_container.h"
#include "controllers/payments_controller.h"

namespace logos::payment_service::http {

/// @brief HTTP server for payment service
/// @details Configures routes and starts Qt HTTP server
class HttpServer {
public:
    explicit HttpServer(application::container::ServiceContainer* container);

    /// Start server on specified port
    bool Start(quint16 port);

    /// Stop server
    void Stop();

private:
    void SetupRoutes();

    QHttpServer server_;
    std::unique_ptr<controllers::PaymentsController> payments_controller_;
};

} // namespace logos::payment_service::http
```

### 5.2 Server Implementation

```cpp
// logos_payment_service_http/http_server.cpp
#include "http_server.h"

namespace logos::payment_service::http {

HttpServer::HttpServer(application::container::ServiceContainer* container) {
    // Create controller with container
    payments_controller_ = std::make_unique<controllers::PaymentsController>(container);

    SetupRoutes();
}

void HttpServer::SetupRoutes() {
    // POST /api/payments/authorize
    server_.route("/api/payments/authorize", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest& request) {
            return payments_controller_->AuthorizePayment(request);
        });

    // GET /api/payments/{id}
    server_.route("/api/payments/<arg>", QHttpServerRequest::Method::Get,
        [this](const QString& id) {
            return payments_controller_->GetPayment(id);
        });
}

bool HttpServer::Start(quint16 port) {
    return server_.listen(QHostAddress::Any, port) != 0;
}

void HttpServer::Stop() {
    // Qt HTTP server stops automatically on destruction
}

} // namespace logos::payment_service::http
```

---

## 6. Main Entry Point

```cpp
// logos_payment_service_http/main.cpp
#include <QCoreApplication>
#include <QDebug>
#include "http_server.h"
#include "application/container/service_container.h"
#include "domain/value_objects/money.h"
#include "adapters/sqlite_payment_repository.h"
#include "adapters/fraud_detection_service.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    // Load configuration (simplified - use proper config library)
    auto maximum_amount = logos::payment_service::domain::value_objects::Money(10000.0, "USD");

    // Create service container
    logos::payment_service::application::container::ServiceContainer container;

    // Register capability implementations (container takes ownership)
    container.Register<domain::abstractions::IPaymentRepository>(
        std::make_unique<adapters::SqlitePaymentRepository>("payments.db"));

    container.Register<domain::abstractions::IFraudDetectionService>(
        std::make_unique<adapters::ThirdPartyFraudDetectionService>(
            "https://fraud-api.example.com", "api-key-here"));

    // Register domain services
    container.Register<domain::services::PaymentAuthorizationService>(
        std::make_unique<domain::services::PaymentAuthorizationService>(
            container.Resolve<domain::abstractions::IFraudDetectionService>(),
            maximum_amount));

    // Register use cases
    container.Register<application::use_cases::AuthorizePaymentUseCase>(
        std::make_unique<application::use_cases::AuthorizePaymentUseCase>(
            container.Resolve<domain::services::PaymentAuthorizationService>(),
            container.Resolve<domain::abstractions::IPaymentRepository>()));

    container.Register<application::use_cases::GetPaymentUseCase>(
        std::make_unique<application::use_cases::GetPaymentUseCase>(
            container.Resolve<domain::abstractions::IPaymentRepository>()));

    // Start HTTP server
    logos::payment_service::http::HttpServer server(&container);

    if (!server.Start(8080)) {
        qCritical() << "Failed to start HTTP server";
        return 1;
    }

    qInfo() << "Payment service listening on http://localhost:8080";
    qInfo() << "Endpoints:";
    qInfo() << "  POST /api/payments/authorize";
    qInfo() << "  GET  /api/payments/{id}";

    return app.exec();
}
```

---

## 7. Documentation Generation

### 7.1 Doxygen Configuration

Create a `Doxyfile` that documents the relationships:

```doxygen
# Doxyfile
PROJECT_NAME           = "Company Payment Service"
OUTPUT_DIRECTORY       = docs
INPUT                  = domain/ application/ http/
RECURSIVE              = YES
EXTRACT_ALL            = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO

# Enable cross-referencing
GENERATE_TAGFILE       = payment_service.tag
REFERENCES_RELATION    = YES
REFERENCED_BY_RELATION = YES

# Custom documentation
HTML_EXTRA_FILES       = api_mapping.md
```

### 7.2 API Mapping Documentation

Create a separate markdown file documenting the HTTP ? Domain mapping:

```markdown
<!-- api_mapping.md -->
# API to Domain Contract Mapping

## POST /api/payments/authorize

**HTTP DTO**: `AuthorizePaymentDto`
**Domain Contract**: `application::contracts::AuthorizePaymentRequest`
**Value Objects**:
- `domain::value_objects::Money` (amount, currency)

**Use Case**: `application::use_cases::AuthorizePaymentUseCase`

### Request Mapping
| HTTP Field   | Domain Type  | Domain Property |
|--------------|--------------|-----------------|
| amount       | double       | Money::amount   |
| currency     | string       | Money::currency |
| merchantId   | string       | merchant_id     |

### Response Mapping
| HTTP Field     | Domain Type     | Domain Property       |
|----------------|-----------------|----------------------|
| paymentId      | string          | PaymentRecord::id    |
| isAuthorized   | bool            | is_authorized        |
| status         | string          | PaymentStatus enum   |
| amount         | double          | Money::GetAmount()   |
| currency       | string          | Money::GetCurrency() |
| declineReason  | string│         | decline_reason       |

## GET /api/payments/{id}

**HTTP DTO**: `PaymentDetailsDto`
**Domain Contract**: `application::contracts::GetPaymentResponse`
**Value Objects**:
- `domain::value_objects::PaymentRecord`
- `domain::value_objects::Money`
- `domain::value_objects::PaymentStatus`

**Use Case**: `application::use_cases::GetPaymentUseCase`
```

### 7.3 Doxygen Comments Strategy

Use `@see` and `@details` tags to create links:

```cpp
/// @brief HTTP request for payment authorization
/// @details This DTO maps to application::contracts::AuthorizePaymentRequest.
///          It represents the HTTP transport format (JSON) and is converted
///          to the domain contract before invoking the use case.
/// 
/// **Domain Contract Mapping:**
/// - Amount + Currency ? domain::value_objects::Money
/// - MerchantId ? string (domain identifier)
/// 
/// @see application::contracts::AuthorizePaymentRequest
/// @see domain::value_objects::Money
/// @see controllers::PaymentsController::AuthorizePayment
struct AuthorizePaymentDto {
    // ...
};
```

This generates HTML documentation with clickable links to domain types.

---

## 8. CMakeLists.txt

```cmake
# HTTP server library
add_executable(logos_payment_service_http
    main.cpp
    http_server.cpp
    controllers/payments_controller.cpp
    models/payment_dto.cpp
)

target_link_libraries(logos_payment_service_http
    logos_payment_service_application
    logos_payment_service_domain
    logos_payment_service_adapters
    Qt6::Core
    Qt6::HttpServer
)

target_include_directories(logos_payment_service_http PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install
install(TARGETS logos_payment_service_http
    RUNTIME DESTINATION bin
)
```

---

## 9. Example Request/Response

### Authorization Request

**POST** `http://localhost:8080/api/payments/authorize`

```bash
curl -X POST http://localhost:8080/api/payments/authorize \
  -H "Content-Type: application/json" \
  -d '{
    "amount": 100.00,
    "currency": "USD",
    "merchantId": "MERCH-001"
  }'
```

**Response** (200 OK)

```json
{
  "paymentId": "550e8400-e29b-41d4-a716-446655440000",
  "isAuthorized": true,
  "status": "Authorized",
  "amount": 100.00,
  "currency": "USD"
}
```

### Get Payment Request

**GET** `http://localhost:8080/api/payments/550e8400-e29b-41d4-a716-446655440000`

```bash
curl http://localhost:8080/api/payments/550e8400-e29b-41d4-a716-446655440000
```

**Response** (200 OK)

```json
{
  "paymentId": "550e8400-e29b-41d4-a716-446655440000",
  "amount": 100.00,
  "currency": "USD",
  "merchantId": "MERCH-001",
  "status": "Authorized",
  "createdAt": "2024-01-15T10:30:00Z"
}
```

---

## 10. Key Principles

### 10.1 Separation of Concerns

- **HTTP DTOs** - Transport format (JSON)
- **Contract Models** - Application boundary
- **Value Objects** - Shared domain concepts

The controller maps between HTTP DTOs and Contract Models.

### 10.2 Thread Lifecycle

```
1. Qt receives HTTP request
   ?
2. Controller method invoked
   ?
3. Parse JSON ? HTTP DTO
   ?
4. Map HTTP DTO ? Contract Model
   ?
5. Invoke Use Case (synchronous)
   ?
6. Map result ? HTTP DTO
   ?
7. Serialize to JSON
   ?
8. Return HTTP Response
   ?
9. Thread ends
```

### 10.3 No Business Logic in Controller

The controller:
- ? Parses JSON
- ? Maps between transport and application models
- ? Returns appropriate HTTP status codes
- ? Handles HTTP-specific concerns
- ? Does NOT make business decisions
- ? Does NOT contain business rules
- ? Does NOT orchestrate operations

### 10.4 Documentation Approach

Since C++ lacks runtime reflection for automatic API documentation:
- **Doxygen comments** in code with `@see` cross-references
- **Separate mapping document** (markdown) explains HTTP ? Domain
- **Generated HTML docs** with clickable links between layers
- **Manual but traceable** - developers can follow links to domain

---

## 11. Summary

This example demonstrates:

1. **Qt HTTP server** - Lightweight HTTP handling
2. **Controller responsibility**: Map HTTP ? Contract Models only
3. **Use Case invocation**: Direct, synchronous call
4. **Thread lifecycle**: Short-lived request-response
5. **Separation**: HTTP DTOs ? Contract Models ? Value Objects
6. **Direct DI**: Service container provides dependencies
7. **Documentation**: Doxygen + manual mapping document

The pattern is simple: receive request ? parse JSON ? map to contract ? invoke use case ? map to JSON ? return response.

---

**See Also:**
- [C++ Implementation (AP-002)](AP-002-Implementation-Cpp.md) - Base implementation this builds on
- [C# HTTP Controller Example](AP-003-HTTP-Controller-CSharp.md) - C# equivalent
- AP-003 - Incoming Implementations (specification)
- AP-007 - Adapter Implementations (for adapters layer)
