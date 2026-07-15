#include "logos_payment_service_http_host/mappings/payment_dto.h"

#include <QJsonValue>

namespace logos::payment::service::http_host::mappings {

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

} // namespace logos::payment::service::http_host::mappings
