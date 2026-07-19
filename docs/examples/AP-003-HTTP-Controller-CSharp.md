# HTTP Host Example - C#

**Version:** 0.1  
**Status:** Draft  
**Applies to:** AP-003 (Incoming Implementations)  
**Builds on:** [AP-002 Implementation - C#](AP-002-Implementation-CSharp.md)

---

## 1. Introduction

This example demonstrates how to add an HTTP controller that receives requests and invokes use cases from the [C# implementation example](AP-002-Implementation-CSharp.md).

> **📦 Compilable Example Available**  
> A fully compilable and runnable version of this example is available at:  
> `Logos.Payment.Service.HttpHost/`  
>
> See the [README](../../examples/csharp/README.md) for build and run instructions.

**Key Points:**
- Controller receives HTTP request
- Maps HTTP payload to Contract Model (Request)
- Invokes Use Case
- Maps result to HTTP response
- Thread lifecycle: Request → Execute → Response → End

---

## 2. Controller Implementation

### 2.1 Payment Controller

**File:** `Logos.Payment.Service.HttpHost/Controllers/PaymentsController.cs`

The `PaymentsController`:
- Receives HTTP requests via ASP.NET Core
- Maps HTTP DTOs to domain contracts (using shared `Money` value object)
- Invokes use cases directly (no mediator)
- Maps results back to HTTP DTOs
- Returns HTTP responses

**Key Methods:**
- `POST /api/payments/authorize` - Authorizes a payment
- `GET /api/payments/{id}` - Retrieves payment details

The controller demonstrates:
- **Thin translation layer** - Only mapping between HTTP and domain
- **Direct invocation** - Use cases injected via constructor
- **Synchronous execution** - Thread: Request -> Execute -> Response -> End

---

## 3. HTTP DTOs (Transport Models)

HTTP DTOs in the controller code are **transport models** - they represent the HTTP-specific format. They are separate from Contract Models but map directly to them.

See the implementation in the controller file for examples of:
- Mapping HTTP DTOs to domain contracts using shared `Money` value object
- XML documentation with `<see cref>` references to domain contracts
- Swagger/OpenAPI integration

**File:** `Logos.Payment.Service.HttpHost/Mappings/PaymentsHttpMapping.cs`

The DTOs defined inline in the controller demonstrate:
- Simple property mapping (HTTP decimal/string -> Domain Money)
- References to domain contracts in XML docs
- Transport-specific validation if needed

---

## 4. Swagger/OpenAPI Configuration

**File:** `Logos.Payment.Service.HttpHost/Configuration/ServiceConfiguration.cs`

The Program.cs configures Swagger with:
- OpenAPI document generation
- XML documentation comments
- Endpoint metadata

To enable XML docs, each project can set `<GenerateDocumentationFile>true</GenerateDocumentationFile>` in its .csproj file.

### 4.1 Result in Swagger UI

The OpenAPI/Swagger documentation will show:

**Authorization Request Schema:**
```
AuthorizePaymentDto
HTTP request for payment authorization

This HTTP DTO maps to AuthorizePaymentRequest.

Domain Contract: AuthorizePaymentRequest
Value Objects Used:
- Money - Amount and Currency

Properties:
  amount (number): Amount in the specified currency
    Example: 100.00
  currency (string): Currency code (ISO 4217)
    Example: USD
  merchantId (string): Merchant identifier
    Example: MERCH-001
```

This approach:
- Documents the **mapping** between HTTP and Domain
- Includes **domain contract references** visible in Swagger
- Shows **Value Objects used**
- Provides **single source of truth** (domain documentation)
- Makes API documentation **traceable** to domain concepts

---

## 5. Program.cs Configuration

```csharp
// Logos.Payment.Service.HttpHost/Program.cs
using Logos.Payment.Service.Core.Application.UseCases;
using Logos.Payment.Service.Core.Domain.Services;
using Logos.Payment.Service.Core.Capabilities.Persistence;
using Logos.Payment.Service.Core.Capabilities.ExternalServices;
using Logos.Payment.Service.Core.SharedKernel.Money;

var builder = WebApplication.CreateBuilder(args);

// Add controllers
builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Domain configuration - injected as parameters
var maximumPaymentAmount = new Money(
    builder.Configuration.GetValue<decimal>("Payment:MaximumAmount"),
    builder.Configuration.GetValue<string>("Payment:Currency") ?? "USD");

builder.Services.AddSingleton(maximumPaymentAmount);

// Register domain services
builder.Services.AddScoped<PaymentAuthorizationService>();

// Register use cases
builder.Services.AddScoped<AuthorizePaymentUseCase>();
builder.Services.AddScoped<GetPaymentUseCase>();

// Register capability implementations (adapters)
builder.Services.AddScoped<IPaymentRepository, EntityFrameworkPaymentRepository>();
builder.Services.AddScoped<IFraudDetectionService, ThirdPartyFraudDetectionService>();

var app = builder.Build();

// Configure HTTP pipeline
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();
app.UseAuthorization();
app.MapControllers();

app.Run();
```

---

## 6. Project Structure

```
Logos.Payment.Service.Core/                    (From AP-002 example)
├── Domain/
├── SharedKernel/
├── Capabilities/
└── Application/

Logos.Payment.Service.HttpHost/                (HTTP incoming)
├── Controllers/
│   └── PaymentsController.cs
├── Mappings/
│   ├── AuthorizePaymentHttpMapping.cs
│   └── PaymentDetailsHttpMapping.cs
├── Configuration/
│   └── ServiceConfiguration.cs
└── Program.cs
```

---

## 7. Key Principles

### 7.1 Separation of Concerns

- **HTTP DTOs** (`AuthorizePaymentDto`) - Transport format (JSON over HTTP)
- **Contract Models** (`AuthorizePaymentRequest`) - Application boundary
- **Shared Kernel types** (`Money`) - Shared business concepts

The controller maps between HTTP DTOs and Contract Models.

### 7.2 Thread Lifecycle

```
1. HTTP Request arrives
   |
2. Controller receives request
   |
3. Map HTTP DTO -> Contract Model
   |
4. Invoke Use Case (synchronous)
   |
5. Map result -> HTTP DTO
   |
6. Return HTTP Response
   |
7. Thread ends
```

The thread is **short-lived** and handles a single request from start to finish.

### 7.3 No Business Logic in Controller

The controller:
- Maps between transport and application models
- Returns appropriate HTTP status codes
- Handles HTTP-specific concerns (routing, status)
- Does NOT make business decisions
- Does NOT contain business rules
- Does NOT orchestrate operations

All business logic is in the Use Case and Domain.

### 7.4 Direct Dependency Injection

The controller receives use cases through constructor injection:
```csharp
public PaymentsController(
    AuthorizePaymentUseCase authorizePaymentUseCase,
    GetPaymentUseCase getPaymentUseCase)
```

No mediator, command bus, or additional abstraction layer. The controller depends directly on the concrete use case classes (as per AP-003 §5.2).

---

## 8. Running the Example

To run the compilable example:

```bash
cd examples/csharp/Logos.PaymentService.WebApi
dotnet run
```

Navigate to `http://localhost:5000` to see the Swagger UI and test the endpoints interactively.

---

## 9. Example Request/Response

### Authorization Request

**POST** `/api/payments/authorize`

```json
{
  "amount": 100.00,
  "currency": "USD",
  "merchantId": "MERCH-001"
}
```

**Response** (200 OK)

```json
{
  "paymentId": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
  "isAuthorized": true,
  "status": "Authorized",
  "amount": 100.00,
  "currency": "USD",
  "declineReason": null
}
```

### Get Payment Request

**GET** `/api/payments/3fa85f64-5717-4562-b3fc-2c963f66afa6`

**Response** (200 OK)

```json
{
  "paymentId": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
  "amount": 100.00,
  "currency": "USD",
  "merchantId": "MERCH-001",
  "status": "Authorized",
  "createdAt": "2024-01-15T10:30:00Z",
  "declineReason": null
}
```

---

## 10. Key Architectural Points

This example demonstrates:

1. **Controller responsibility**: Map HTTP -> Contract Models, nothing more
2. **Use Case invocation**: Direct, synchronous call
3. **Thread lifecycle**: Short-lived request-response (Request -> Execute -> Response -> End)
4. **Separation**: HTTP DTOs -> Contract Models -> Value Objects
5. **Dependency injection**: Direct constructor injection of use cases
6. **No abstraction layers**: No mediator, command bus, or service locator

The pattern is simple: **receive request -> map to contract -> invoke use case -> map to response -> return**.

---

**See Also:**
- [C# Implementation (AP-002)](AP-002-Implementation-CSharp.md) - Base implementation this builds on
- [Compilable Example README](../../examples/csharp/README.md) - Full build and run instructions
- AP-003 - Incoming Implementations (specification)
- AP-007 - Adapter Implementations (for Infrastructure layer)
