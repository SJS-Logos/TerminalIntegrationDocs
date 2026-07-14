#pragma once

#include "logos_payment_service_core/application/contracts/get_payment_response.h"
#include "logos_payment_service_core/capabilities/payment_repository.h"
#include <optional>
#include <string>

namespace logos::payment::service::core::application::use_cases {

/// Use case for retrieving payment records.
class GetPaymentUseCase {
public:
    explicit GetPaymentUseCase(capabilities::IPaymentRepository* repository);

    std::optional<contracts::GetPaymentResponse> Execute(
        const std::string& payment_id);

private:
    capabilities::IPaymentRepository* repository_;  // Non-owning
};

} // namespace logos::payment::service::core::application::use_cases
