#pragma once

#include "mypaymentservice_core/shared_kernel/money.h"
#include "mypaymentservice_core/shared_kernel/payment_status.h"
#include <string>
#include <optional>

namespace mypaymentservice::core::application::contracts {

/// Contract model that references shared Value Objects.
struct AuthorizePaymentResponse {
    std::string payment_id;
    bool is_authorized;
    shared_kernel::PaymentStatus status;
    shared_kernel::Money amount;
    std::optional<std::string> decline_reason;
};

} // namespace mypaymentservice::core::application::contracts
