#pragma once

#include "logos_payment_core/shared_kernel/payment_record.h"

namespace logos::payment::core::application::contracts {

/// Contract model for retrieving payment records.
using GetPaymentResponse = shared_kernel::PaymentRecord;

} // namespace logos::payment::core::application::contracts
