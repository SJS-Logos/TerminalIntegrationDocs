#pragma once

namespace logos::payment_service::domain::value_objects {

/// Shared Value Object representing payment authorization status.
enum class PaymentStatus {
    Pending,
    Authorized,
    Declined
};

} // namespace logos::payment_service::domain::value_objects
