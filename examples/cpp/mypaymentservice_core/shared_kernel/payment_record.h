#pragma once

#include "money.h"
#include "payment_status.h"
#include <string>
#include <chrono>
#include <optional>

namespace mypaymentservice::core::shared_kernel {

/// Value Object representing a payment record.
/// The identity (id) is assigned by the repository/database.
struct PaymentRecord {
    std::string id;
    Money amount;
    std::string merchant_id;
    PaymentStatus status;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::string> decline_reason;

    // Default constructor for container compatibility
    PaymentRecord()
        : id("")
        , amount(Money::Zero("USD"))
        , merchant_id("")
        , status(PaymentStatus::Pending)
        , created_at(std::chrono::system_clock::now())
        , decline_reason(std::nullopt) {}

    PaymentRecord(
        const std::string& id_,
        const Money& amount_,
        const std::string& merchant_id_,
        PaymentStatus status_,
        std::chrono::system_clock::time_point created_at_,
        const std::optional<std::string>& decline_reason_ = std::nullopt)
        : id(id_)
        , amount(amount_)
        , merchant_id(merchant_id_)
        , status(status_)
        , created_at(created_at_)
        , decline_reason(decline_reason_) {}
};

} // namespace mypaymentservice::core::shared_kernel
