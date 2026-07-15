#include "logos_payment_service_http_host/controllers/payments_controller.h"

#include "logos_payment_service_http_host/mappings/payment_dto.h"
#include "logos_payment_service_core/application/contracts/authorize_payment_request.h"
#include "logos_payment_service_core/shared_kernel/money.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <chrono>

namespace logos::payment::service::http_host::controllers {

namespace {

/// Convert Money minor units to a display double (major units).
double ToMajorUnits(const core::shared_kernel::Money& money) {
    return static_cast<double>(money.GetMinorUnits()) / 100.0;
}

QHttpServerResponse JsonError(const QString& message,
                              QHttpServerResponse::StatusCode code) {
    QJsonObject body{{"error", message}};
    return QHttpServerResponse("application/json",
                               QJsonDocument(body).toJson(),
                               code);
}

} // namespace

PaymentsController::PaymentsController(
    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case,
    core::application::use_cases::GetPaymentUseCase* get_use_case)
    : authorize_use_case_(authorize_use_case)
    , get_use_case_(get_use_case) {
}

QHttpServerResponse PaymentsController::AuthorizePayment(const QHttpServerRequest& request) {
    QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (!doc.isObject()) {
        return JsonError("Invalid JSON", QHttpServerResponse::StatusCode::BadRequest);
    }

    auto dto_opt = mappings::AuthorizePaymentDto::FromJson(doc.object());
    if (!dto_opt) {
        return JsonError("Missing required fields",
                         QHttpServerResponse::StatusCode::BadRequest);
    }
    const auto& dto = *dto_opt;

    try {
        // Map HTTP DTO -> Contract Model (using shared Value Objects).
        core::application::contracts::AuthorizePaymentRequest use_case_request{
            core::shared_kernel::Money(std::to_string(dto.amount),
                                       dto.currency.toStdString()),
            dto.merchant_id.toStdString()
        };

        // Execute use case (synchronous, short-lived thread).
        auto result = authorize_use_case_->Execute(use_case_request);

        // Map result -> HTTP DTO.
        mappings::PaymentAuthorizationDto response_dto;
        response_dto.payment_id = QString::fromStdString(result.payment_id);
        response_dto.is_authorized = result.is_authorized;
        response_dto.status = PaymentStatusToString(result.status);
        response_dto.amount = ToMajorUnits(result.amount);
        response_dto.currency = QString::fromStdString(result.amount.GetCurrency());
        if (result.decline_reason) {
            response_dto.decline_reason = QString::fromStdString(*result.decline_reason);
        }

        return QHttpServerResponse("application/json",
                                   QJsonDocument(response_dto.ToJson()).toJson(),
                                   QHttpServerResponse::StatusCode::Ok);
    } catch (const std::exception& e) {
        return JsonError(QString::fromStdString(e.what()),
                         QHttpServerResponse::StatusCode::BadRequest);
    }
}

QHttpServerResponse PaymentsController::GetPayment(const QString& payment_id) {
    auto result = get_use_case_->Execute(payment_id.toStdString());
    if (!result) {
        return JsonError("Payment not found",
                         QHttpServerResponse::StatusCode::NotFound);
    }

    mappings::PaymentDetailsDto response_dto;
    response_dto.payment_id = QString::fromStdString(result->id);
    response_dto.amount = ToMajorUnits(result->amount);
    response_dto.currency = QString::fromStdString(result->amount.GetCurrency());
    response_dto.merchant_id = QString::fromStdString(result->merchant_id);
    response_dto.status = PaymentStatusToString(result->status);
    response_dto.created_at = QDateTime::fromSecsSinceEpoch(
        std::chrono::system_clock::to_time_t(result->created_at));
    if (result->decline_reason) {
        response_dto.decline_reason = QString::fromStdString(*result->decline_reason);
    }

    return QHttpServerResponse("application/json",
                               QJsonDocument(response_dto.ToJson()).toJson(),
                               QHttpServerResponse::StatusCode::Ok);
}

QString PaymentsController::PaymentStatusToString(
    core::shared_kernel::PaymentStatus status) {
    switch (status) {
        case core::shared_kernel::PaymentStatus::Pending:
            return "Pending";
        case core::shared_kernel::PaymentStatus::Authorized:
            return "Authorized";
        case core::shared_kernel::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment::service::http_host::controllers
