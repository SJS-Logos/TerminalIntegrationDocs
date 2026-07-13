#pragma once

#include "logos_payment_service_domain/value_objects/payment_record.h"
#include "logos_payment_service_domain/value_objects/money.h"
#include "logos_payment_service_domain/value_objects/payment_status.h"
#include <memory>
#include <optional>
#include <string>

namespace logos::payment_service::domain::abstractions {

/// Capability: Persistence expressed in domain terms.
/// Responsible for identity generation and state management.
class IPaymentRepository {
public:
    virtual ~IPaymentRepository() = default;

    virtual std::optional<value_objects::PaymentRecord> GetById(
        const std::string& id) = 0;

    /// Saves a payment record. The repository assigns the id.
    virtual value_objects::PaymentRecord Save(
        const value_objects::Money& amount,
        const std::string& merchant_id,
        value_objects::PaymentStatus status,
        const std::optional<std::string>& decline_reason) = 0;
};

} // namespace logos::payment_service::domain::abstractions
