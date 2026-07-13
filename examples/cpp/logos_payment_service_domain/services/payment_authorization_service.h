#pragma once

#include "logos_payment_service_domain/value_objects/money.h"
#include "logos_payment_service_domain/value_objects/payment_status.h"
#include "logos_payment_service_domain/abstractions/fraud_detection_service.h"
#include <memory>
#include <optional>
#include <string>

namespace logos::payment_service::domain::services {

/// Result of payment authorization business logic
struct PaymentAuthorizationResult {
    bool is_authorized;
    value_objects::PaymentStatus status;
    std::optional<std::string> decline_reason;
};

/// Stateless service that performs payment authorization business logic.
/// Works with Value Objects; state persistence is handled by repository.
class PaymentAuthorizationService {
public:
    explicit PaymentAuthorizationService(
        abstractions::IFraudDetectionService* fraud_detection,
        const value_objects::Money& maximum_amount);

    PaymentAuthorizationResult Authorize(
        const value_objects::Money& amount,
        const std::string& merchant_id);

private:
    abstractions::IFraudDetectionService* fraud_detection_;  // Non-owning
    value_objects::Money maximum_amount_;
};

} // namespace logos::payment_service::domain::services
