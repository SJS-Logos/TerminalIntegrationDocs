#pragma once

#include "logos_payment_service_domain/value_objects/payment_record.h"

namespace logos::payment_service::application::contracts {

/// Contract model for retrieving payment records.
using GetPaymentResponse = domain::value_objects::PaymentRecord;

} // namespace logos::payment_service::application::contracts
