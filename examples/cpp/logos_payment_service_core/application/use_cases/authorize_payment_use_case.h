#pragma once

#include "logos_payment_service_core/application/contracts/authorize_payment_request.h"
#include "logos_payment_service_core/application/contracts/authorize_payment_response.h"
#include "logos_payment_service_core/domain/services/payment_authorization_service.h"
#include "logos_payment_service_core/capabilities/payment_repository.h"

namespace logos::payment::service::core::application::use_cases {

/// Use case that orchestrates payment authorization.
/// Domain service performs business logic.
/// Repository manages state and identity.
class AuthorizePaymentUseCase {
public:
    AuthorizePaymentUseCase(
        domain::services::PaymentAuthorizationService* auth_service,
        capabilities::IPaymentRepository* repository);

    contracts::AuthorizePaymentResponse Execute(
        const contracts::AuthorizePaymentRequest& request);

private:
    domain::services::PaymentAuthorizationService* auth_service_;  // Non-owning
    capabilities::IPaymentRepository* repository_;                 // Non-owning
};

} // namespace logos::payment::service::core::application::use_cases
