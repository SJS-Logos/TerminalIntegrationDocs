#pragma once

#include "mypaymentservice_core/shared_kernel/money.h"
#include <string>

namespace mypaymentservice::core::application::contracts {

/// Contract model that references shared Value Objects.
/// No duplication - Money is defined once in SharedKernel.
struct AuthorizePaymentRequest {
    shared_kernel::Money amount;
    std::string merchant_id;
};

} // namespace mypaymentservice::core::application::contracts
