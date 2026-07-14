#pragma once

#include "logos_payment_service_core/shared_kernel/money.h"
#include <string>

namespace logos::payment::service::core::application::contracts {

/// Contract model that references shared Value Objects.
/// No duplication - Money is defined once in SharedKernel.
struct AuthorizePaymentRequest {
    shared_kernel::Money amount;
    std::string merchant_id;
};

} // namespace logos::payment::service::core::application::contracts
