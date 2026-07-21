#pragma once

#include "mypaymentservice_core/application/contracts/get_payment_response.h"
#include "mypaymentservice_core/capabilities/payment_repository.h"
#include <optional>
#include <string>

namespace mypaymentservice::core::application::use_cases {

/// Use case for retrieving payment records.
class GetPaymentUseCase {
public:
    explicit GetPaymentUseCase(capabilities::IPaymentRepository* repository);

    std::optional<contracts::GetPaymentResponse> Execute(
        const std::string& payment_id);

private:
    capabilities::IPaymentRepository* repository_;  // Non-owning
};

} // namespace mypaymentservice::core::application::use_cases
