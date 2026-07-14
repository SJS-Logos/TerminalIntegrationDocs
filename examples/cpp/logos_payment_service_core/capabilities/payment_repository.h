#pragma once

#include "logos_payment_service_core/shared_kernel/payment_record.h"
#include "logos_payment_service_core/shared_kernel/money.h"
#include "logos_payment_service_core/shared_kernel/payment_status.h"
#include <memory>
#include <optional>
#include <string>

namespace logos::payment::service::core::capabilities {

/// Capability: Persistence expressed in domain terms.
/// Responsible for identity generation and state management.
class IPaymentRepository {
public:
    virtual ~IPaymentRepository() = default;

    virtual std::optional<shared_kernel::PaymentRecord> GetById(
        const std::string& id) = 0;

    /// Saves a payment record. The repository assigns the id.
    virtual shared_kernel::PaymentRecord Save(
        const shared_kernel::Money& amount,
        const std::string& merchant_id,
        shared_kernel::PaymentStatus status,
        const std::optional<std::string>& decline_reason) = 0;
};

} // namespace logos::payment::service::core::capabilities
