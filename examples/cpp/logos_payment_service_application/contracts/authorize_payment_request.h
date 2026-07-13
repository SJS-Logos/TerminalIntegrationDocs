#pragma once

#include "logos_payment_service_domain/value_objects/money.h"
#include <string>

namespace logos::payment_service::application::contracts {

/// Contract model that references shared Value Objects.
/// No duplication - Money is defined once in Domain.
struct AuthorizePaymentRequest {
    domain::value_objects::Money amount;
    std::string merchant_id;
};

} // namespace logos::payment_service::application::contracts
