#pragma once

#include "logos_payment_core/shared_kernel/money.h"
#include "logos_payment_core/shared_kernel/payment_status.h"
#include "logos_payment_core/capabilities/fraud_detection_service.h"
#include <memory>
#include <optional>
#include <string>

namespace logos::payment::core::domain::services {

/// Result of payment authorization business logic
struct PaymentAuthorizationResult {
    bool is_authorized;
    shared_kernel::PaymentStatus status;
    std::optional<std::string> decline_reason;
};

/// Stateless service that performs payment authorization business logic.
/// Works with Value Objects; state persistence is handled by repository.
class PaymentAuthorizationService {
public:
    explicit PaymentAuthorizationService(
        capabilities::IFraudDetectionService* fraud_detection,
        const shared_kernel::Money& maximum_amount);

    PaymentAuthorizationResult Authorize(
        const shared_kernel::Money& amount,
        const std::string& merchant_id);

private:
    capabilities::IFraudDetectionService* fraud_detection_;  // Non-owning
    shared_kernel::Money maximum_amount_;
};

} // namespace logos::payment::core::domain::services
