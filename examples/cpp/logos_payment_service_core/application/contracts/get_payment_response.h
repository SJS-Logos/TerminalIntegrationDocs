#pragma once

#include "logos_payment_service_core/shared_kernel/payment_record.h"

namespace logos::payment::service::core::application::contracts {

/// Contract model for retrieving payment records.
using GetPaymentResponse = shared_kernel::PaymentRecord;

} // namespace logos::payment::service::core::application::contracts
