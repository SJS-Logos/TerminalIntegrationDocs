#pragma once

namespace mypaymentservice::core::shared_kernel {

/// Shared Kernel Value Object representing payment authorization status.
enum class PaymentStatus {
    Pending,
    Authorized,
    Declined
};

} // namespace mypaymentservice::core::shared_kernel
