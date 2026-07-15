#pragma once

#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QString>

#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"
#include "logos_payment_service_core/application/use_cases/get_payment_use_case.h"
#include "logos_payment_service_core/shared_kernel/payment_status.h"

namespace logos::payment::service::http_host::controllers {

/// @brief HTTP controller for payment operations.
/// @details Handles HTTP requests for payment authorization and retrieval.
///          Maps HTTP DTOs to Application contracts and invokes use cases.
///
/// Thread lifecycle: Request -> Map DTO -> Invoke Use Case -> Map Result -> Response -> End
///
/// @note This controller contains NO business logic. All business decisions
///       are made in the Domain layer (AP-003 §7.1).
///
/// @see core::application::use_cases::AuthorizePaymentUseCase
/// @see core::application::use_cases::GetPaymentUseCase
class PaymentsController {
public:
    PaymentsController(
        core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case,
        core::application::use_cases::GetPaymentUseCase* get_use_case);

    /// @brief POST /api/payments/authorize
    /// @details Authorizes a payment request.
    /// @param request HTTP request containing payment details (JSON).
    /// @return HTTP 200 with authorization result, or 400 on invalid request.
    QHttpServerResponse AuthorizePayment(const QHttpServerRequest& request);

    /// @brief GET /api/payments/{id}
    /// @details Retrieves payment details by ID.
    /// @param payment_id Payment identifier.
    /// @return HTTP 200 with payment details, or 404 if not found.
    QHttpServerResponse GetPayment(const QString& payment_id);

private:
    static QString PaymentStatusToString(core::shared_kernel::PaymentStatus status);

    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case_;  // Non-owning
    core::application::use_cases::GetPaymentUseCase* get_use_case_;              // Non-owning
};

} // namespace logos::payment::service::http_host::controllers
