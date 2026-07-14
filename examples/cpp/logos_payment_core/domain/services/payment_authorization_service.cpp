#include "logos_payment_core/domain/services/payment_authorization_service.h"

namespace logos::payment::core::domain::services {

PaymentAuthorizationService::PaymentAuthorizationService(
    capabilities::IFraudDetectionService* fraud_detection,
    const shared_kernel::Money& maximum_amount)
    : fraud_detection_(fraud_detection)
    , maximum_amount_(maximum_amount) {
}

PaymentAuthorizationResult PaymentAuthorizationService::Authorize(
    const shared_kernel::Money& amount,
    const std::string& merchant_id) {

    // Business rule: Amount must be positive
    if (!amount.IsPositive()) {
        return PaymentAuthorizationResult{
            false,
            shared_kernel::PaymentStatus::Declined,
            "Amount must be positive"
        };
    }

    // Business rule: Check for fraud (pass minor units for precise calculation)
    bool is_fraudulent = fraud_detection_->IsFraudulent(
        amount.GetMinorUnits(),
        amount.GetCurrency(),
        merchant_id);

    if (is_fraudulent) {
        return PaymentAuthorizationResult{
            false,
            shared_kernel::PaymentStatus::Declined,
            "Suspected fraud"
        };
    }

    // Business rule: Amount limits
    if (amount.GetCurrency() == maximum_amount_.GetCurrency() &&
        amount.IsGreaterThan(maximum_amount_)) {
        return PaymentAuthorizationResult{
            false,
            shared_kernel::PaymentStatus::Declined,
            "Amount exceeds maximum limit"
        };
    }

    return PaymentAuthorizationResult{
        true,
        shared_kernel::PaymentStatus::Authorized,
        std::nullopt
    };
}

} // namespace logos::payment::core::domain::services
