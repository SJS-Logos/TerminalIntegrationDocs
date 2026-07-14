#include "logos_payment_service_infrastructure/simple_fraud_detection_service.h"

namespace logos::payment::service::infrastructure {

SimpleFraudDetectionService::SimpleFraudDetectionService() {}

bool SimpleFraudDetectionService::IsFraudulent(
    int64_t amount_cents,
    const std::string& currency,
    const std::string& merchant_id) {

    // Simple rule: amounts over $5000 (500000 cents) are suspicious
    return amount_cents > 500000;
}

} // namespace logos::payment::service::infrastructure
