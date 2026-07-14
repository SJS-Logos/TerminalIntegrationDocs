#pragma once

#include "logos_payment_service_core/capabilities/fraud_detection_service.h"
#include <cstdint>

namespace logos::payment::service::infrastructure {

/// Simple fraud detection service for testing/demo purposes.
/// Flags amounts over $5000 (500000 cents) as fraudulent.
class SimpleFraudDetectionService : public core::capabilities::IFraudDetectionService {
public:
    SimpleFraudDetectionService();

    bool IsFraudulent(
        int64_t amount_cents,
        const std::string& currency,
        const std::string& merchant_id) override;
};

} // namespace logos::payment::service::infrastructure
