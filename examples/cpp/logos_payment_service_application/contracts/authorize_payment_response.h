#pragma once

#include "logos_payment_service_domain/value_objects/money.h"
#include "logos_payment_service_domain/value_objects/payment_status.h"
#include <string>
#include <optional>

namespace logos::payment_service::application::contracts {

/// Contract model that references shared Value Objects.
struct AuthorizePaymentResponse {
    std::string payment_id;
    bool is_authorized;
    domain::value_objects::PaymentStatus status;
    domain::value_objects::Money amount;
    std::optional<std::string> decline_reason;
};

} // namespace logos::payment_service::application::contracts
