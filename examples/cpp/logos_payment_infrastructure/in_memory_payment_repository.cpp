#include "logos_payment_infrastructure/in_memory_payment_repository.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace logos::payment::infrastructure {

InMemoryPaymentRepository::InMemoryPaymentRepository() : next_id_(1) {}

std::optional<core::shared_kernel::PaymentRecord>
InMemoryPaymentRepository::GetById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = payments_.find(id);
    if (it != payments_.end()) {
        return it->second;
    }
    return std::nullopt;
}

core::shared_kernel::PaymentRecord
InMemoryPaymentRepository::Save(
    const core::shared_kernel::Money& amount,
    const std::string& merchant_id,
    core::shared_kernel::PaymentStatus status,
    const std::optional<std::string>& decline_reason) {

    std::lock_guard<std::mutex> lock(mutex_);

    // Generate ID
    std::stringstream ss;
    ss << "PAY-" << std::setw(6) << std::setfill('0') << next_id_++;
    std::string id = ss.str();

    // Create record using constructor
    core::shared_kernel::PaymentRecord record(
        id,
        amount,
        merchant_id,
        status,
        std::chrono::system_clock::now(),
        decline_reason
    );

    // Store
    payments_[id] = record;

    return record;
}

} // namespace logos::payment::infrastructure
