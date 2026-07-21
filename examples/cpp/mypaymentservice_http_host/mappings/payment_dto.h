#pragma once

#include <QJsonObject>
#include <QString>
#include <QDateTime>
#include <optional>

namespace mypaymentservice::http_host::mappings {

/// @brief HTTP request for payment authorization.
/// @details Transport model that maps to
///          core::application::contracts::AuthorizePaymentRequest.
/// @see core::application::contracts::AuthorizePaymentRequest
/// @see core::shared_kernel::Money
struct AuthorizePaymentDto {
    double amount = 0.0;
    QString currency;
    QString merchant_id;

    /// Parse from JSON. Returns std::nullopt when required fields are missing.
    static std::optional<AuthorizePaymentDto> FromJson(const QJsonObject& json);

    /// Convert to JSON.
    QJsonObject ToJson() const;
};

/// @brief HTTP response for payment authorization.
/// @details Transport model that maps from
///          core::application::contracts::AuthorizePaymentResponse.
/// @see core::application::contracts::AuthorizePaymentResponse
/// @see core::shared_kernel::PaymentStatus
struct PaymentAuthorizationDto {
    QString payment_id;
    bool is_authorized = false;
    QString status;
    double amount = 0.0;
    QString currency;
    std::optional<QString> decline_reason;

    /// Convert to JSON.
    QJsonObject ToJson() const;
};

/// @brief HTTP response for payment details.
/// @details Transport model that maps from
///          core::application::contracts::GetPaymentResponse.
/// @see core::application::contracts::GetPaymentResponse
struct PaymentDetailsDto {
    QString payment_id;
    double amount = 0.0;
    QString currency;
    QString merchant_id;
    QString status;
    QDateTime created_at;
    std::optional<QString> decline_reason;

    /// Convert to JSON.
    QJsonObject ToJson() const;
};

} // namespace mypaymentservice::http_host::mappings
