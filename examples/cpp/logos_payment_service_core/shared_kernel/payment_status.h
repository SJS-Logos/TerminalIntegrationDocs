#pragma once

namespace logos::payment::service::core::shared_kernel {

/// Shared Kernel Value Object representing payment authorization status.
enum class PaymentStatus {
    Pending,
    Authorized,
    Declined
};

} // namespace logos::payment::service::core::shared_kernel
