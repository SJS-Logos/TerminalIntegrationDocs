#include "logos_payment_service_domain/services/payment_authorization_service.h"

namespace logos::payment_service::domain::services {

PaymentAuthorizationService::PaymentAuthorizationService(
    abstractions::IFraudDetectionService* fraud_detection,
    const value_objects::Money& maximum_amount)
    : fraud_detection_(fraud_detection)
    , maximum_amount_(maximum_amount) {
}

PaymentAuthorizationResult PaymentAuthorizationService::Authorize(
    const value_objects::Money& amount,
    const std::string& merchant_id) {

    // Business rule: Amount must be positive
    if (!amount.IsPositive()) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Amount must be positive"
        };
    }

    // Business rule: Check for fraud (pass cents for precise calculation)
    bool is_fraudulent = fraud_detection_->IsFraudulent(
        amount.GetCents(),
        amount.GetCurrency(),
        merchant_id);

    if (is_fraudulent) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Suspected fraud"
        };
    }

    // Business rule: Amount limits
    if (amount.GetCurrency() == maximum_amount_.GetCurrency() &&
        amount.IsGreaterThan(maximum_amount_)) {
        return PaymentAuthorizationResult{
            false,
            value_objects::PaymentStatus::Declined,
            "Amount exceeds maximum limit"
        };
    }

    return PaymentAuthorizationResult{
        true,
        value_objects::PaymentStatus::Authorized,
        std::nullopt
    };
}

} // namespace logos::payment_service::domain::services
