#include "logos_payment_service_adapters/in_memory_payment_repository.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace logos::payment_service::adapters {

InMemoryPaymentRepository::InMemoryPaymentRepository() : next_id_(1) {}

std::optional<domain::value_objects::PaymentRecord> 
InMemoryPaymentRepository::GetById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = payments_.find(id);
    if (it != payments_.end()) {
        return it->second;
    }
    return std::nullopt;
}

domain::value_objects::PaymentRecord 
InMemoryPaymentRepository::Save(
    const domain::value_objects::Money& amount,
    const std::string& merchant_id,
    domain::value_objects::PaymentStatus status,
    const std::optional<std::string>& decline_reason) {

    std::lock_guard<std::mutex> lock(mutex_);

    // Generate ID
    std::stringstream ss;
    ss << "PAY-" << std::setw(6) << std::setfill('0') << next_id_++;
    std::string id = ss.str();

    // Create record using constructor
    domain::value_objects::PaymentRecord record(
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

} // namespace logos::payment_service::adapters
