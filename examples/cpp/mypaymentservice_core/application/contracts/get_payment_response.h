#pragma once

#include "mypaymentservice_core/shared_kernel/payment_record.h"

namespace mypaymentservice::core::application::contracts {

/// Contract model for retrieving payment records.
using GetPaymentResponse = shared_kernel::PaymentRecord;

} // namespace mypaymentservice::core::application::contracts
