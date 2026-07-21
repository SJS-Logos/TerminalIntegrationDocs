#pragma once

#include "mypaymentservice_core/capabilities/payment_repository.h"
#include <unordered_map>
#include <mutex>

namespace mypaymentservice::infrastructure {

/// In-memory implementation of payment repository for testing/demo purposes.
class InMemoryPaymentRepository : public core::capabilities::IPaymentRepository {
public:
    InMemoryPaymentRepository();

    std::optional<core::shared_kernel::PaymentRecord> GetById(
        const std::string& id) override;

    core::shared_kernel::PaymentRecord Save(
        const core::shared_kernel::Money& amount,
        const std::string& merchant_id,
        core::shared_kernel::PaymentStatus status,
        const std::optional<std::string>& decline_reason) override;

private:
    std::unordered_map<std::string, core::shared_kernel::PaymentRecord> payments_;
    int next_id_;
    std::mutex mutex_;
};

} // namespace mypaymentservice::infrastructure
