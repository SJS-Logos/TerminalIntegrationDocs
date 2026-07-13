#pragma once

#include "logos_payment_service_application/contracts/get_payment_response.h"
#include "logos_payment_service_domain/abstractions/payment_repository.h"
#include <optional>
#include <string>

namespace logos::payment_service::application::use_cases {

/// Use case for retrieving payment records.
class GetPaymentUseCase {
public:
    explicit GetPaymentUseCase(domain::abstractions::IPaymentRepository* repository);

    std::optional<contracts::GetPaymentResponse> Execute(
        const std::string& payment_id);

private:
    domain::abstractions::IPaymentRepository* repository_;  // Non-owning
};

} // namespace logos::payment_service::application::use_cases
