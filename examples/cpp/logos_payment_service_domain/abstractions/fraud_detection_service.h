#pragma once

#include <string>
#include <cstdint>

namespace logos::payment_service::domain::abstractions {

/// Capability: Fraud detection expressed in domain terms.
class IFraudDetectionService {
public:
    virtual ~IFraudDetectionService() = default;

    /// Check if a transaction is fraudulent
    /// @param amount_cents Amount in cents (smallest currency unit)
    /// @param currency Currency code (e.g., "USD")
    /// @param merchant_id Merchant identifier
    virtual bool IsFraudulent(
        int64_t amount_cents,
        const std::string& currency,
        const std::string& merchant_id) = 0;
};

} // namespace logos::payment_service::domain::abstractions
