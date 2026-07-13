#pragma once

#include "logos_payment_service_application/contracts/authorize_payment_request.h"
#include "logos_payment_service_application/contracts/authorize_payment_response.h"
#include "logos_payment_service_domain/services/payment_authorization_service.h"
#include "logos_payment_service_domain/abstractions/payment_repository.h"

namespace logos::payment_service::application::use_cases {

/// Use case that orchestrates payment authorization.
/// Domain service performs business logic.
/// Repository manages state and identity.
class AuthorizePaymentUseCase {
public:
    AuthorizePaymentUseCase(
        domain::services::PaymentAuthorizationService* auth_service,
        domain::abstractions::IPaymentRepository* repository);

    contracts::AuthorizePaymentResponse Execute(
        const contracts::AuthorizePaymentRequest& request);

private:
    domain::services::PaymentAuthorizationService* auth_service_;  // Non-owning
    domain::abstractions::IPaymentRepository* repository_;         // Non-owning
};

} // namespace logos::payment_service::application::use_cases
