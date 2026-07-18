#include "logos_payment_service_rabbitmq_host/mappings/message_mapping.h"

#include "logos_payment_service_core/shared_kernel/money.h"
#include "logos_payment_service_core/shared_kernel/payment_status.h"

#include <nlohmann/json.hpp>

#include <sstream>
#include <iomanip>
#include <ctime>

namespace logos::payment::service::rabbitmq_host::mappings {

namespace {

using json = nlohmann::json;

/// Convert Money to a fixed 2-decimal major-units string, e.g. "100.00".
std::string MoneyToMajorUnitsString(const core::shared_kernel::Money& money) {
    const int64_t minor = money.GetMinorUnits();
    const int64_t major = minor / 100;
    const int64_t remainder = minor < 0 ? -(minor % 100) : (minor % 100);

    std::ostringstream oss;
    oss << major << '.' << std::setw(2) << std::setfill('0') << remainder;
    return oss.str();
}

std::string PaymentStatusToString(core::shared_kernel::PaymentStatus status) {
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

/// Read an optional string field from JSON (missing or null -> nullopt).
std::optional<std::string> ReadOptionalString(const json& j, const char* key) {
    if (!j.contains(key) || j[key].is_null()) {
        return std::nullopt;
    }
    return j[key].get<std::string>();
}

} // namespace

std::optional<messages::AuthorizePaymentCommand>
MessageMapping::ParseAuthorizeCommand(const std::string& json_payload) {
    json j = json::parse(json_payload, nullptr, /*allow_exceptions=*/false);
    if (j.is_discarded() || !j.is_object()) {
        return std::nullopt;
    }

    // Required fields.
    if (!j.contains("amount") || !j.contains("currency") || !j.contains("merchantId")) {
        return std::nullopt;
    }

    messages::AuthorizePaymentCommand command;
    command.message_id = j.value("messageId", std::string{});

    // Accept amount as either a string ("100.00") or a JSON number (100.00).
    if (j["amount"].is_string()) {
        command.amount = j["amount"].get<std::string>();
    } else if (j["amount"].is_number()) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << j["amount"].get<double>();
        command.amount = oss.str();
    } else {
        return std::nullopt;
    }

    command.currency = j["currency"].get<std::string>();
    command.merchant_id = j["merchantId"].get<std::string>();
    command.correlation_id = ReadOptionalString(j, "correlationId");

    return command;
}

core::application::contracts::AuthorizePaymentRequest
MessageMapping::ToContract(const messages::AuthorizePaymentCommand& command) {
    // Map transport message -> Contract Model (using shared Value Objects).
    return core::application::contracts::AuthorizePaymentRequest{
        core::shared_kernel::Money(command.amount, command.currency),
        command.merchant_id
    };
}

messages::PaymentAuthorizedEvent
MessageMapping::ToEvent(
    const core::application::contracts::AuthorizePaymentResponse& result,
    const std::optional<std::string>& correlation_id) {

    messages::PaymentAuthorizedEvent event;
    event.payment_id = result.payment_id;
    event.is_authorized = result.is_authorized;
    event.status = PaymentStatusToString(result.status);
    event.amount = MoneyToMajorUnitsString(result.amount);
    event.currency = result.amount.GetCurrency();
    event.decline_reason = result.decline_reason;
    event.correlation_id = correlation_id;

    // ISO 8601 UTC timestamp.
    const std::time_t now = std::time(nullptr);
    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
    event.authorized_at = oss.str();

    return event;
}

std::string
MessageMapping::SerializeEvent(const messages::PaymentAuthorizedEvent& event) {
    json j;
    j["paymentId"] = event.payment_id;
    j["isAuthorized"] = event.is_authorized;
    j["status"] = event.status;
    j["amount"] = event.amount;
    j["currency"] = event.currency;
    if (event.decline_reason) {
        j["declineReason"] = *event.decline_reason;
    } else {
        j["declineReason"] = nullptr;
    }
    j["authorizedAt"] = event.authorized_at;
    if (event.correlation_id) {
        j["correlationId"] = *event.correlation_id;
    } else {
        j["correlationId"] = nullptr;
    }
    return j.dump();
}

} // namespace logos::payment::service::rabbitmq_host::mappings
