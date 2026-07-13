#pragma once

#include "logos_payment_service_domain/abstractions/payment_repository.h"
#include <unordered_map>
#include <mutex>

namespace logos::payment_service::adapters {

/// In-memory implementation of payment repository for testing/demo purposes.
class InMemoryPaymentRepository : public domain::abstractions::IPaymentRepository {
public:
    InMemoryPaymentRepository();

    std::optional<domain::value_objects::PaymentRecord> GetById(
        const std::string& id) override;

    domain::value_objects::PaymentRecord Save(
        const domain::value_objects::Money& amount,
        const std::string& merchant_id,
        domain::value_objects::PaymentStatus status,
        const std::optional<std::string>& decline_reason) override;

private:
    std::unordered_map<std::string, domain::value_objects::PaymentRecord> payments_;
    int next_id_;
    std::mutex mutex_;
};

} // namespace logos::payment_service::adapters
