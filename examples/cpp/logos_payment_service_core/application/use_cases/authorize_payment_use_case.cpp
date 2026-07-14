#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"

namespace logos::payment::service::core::application::use_cases {

AuthorizePaymentUseCase::AuthorizePaymentUseCase(
    domain::services::PaymentAuthorizationService* auth_service,
    capabilities::IPaymentRepository* repository)
    : auth_service_(auth_service)
    , repository_(repository) {
}

contracts::AuthorizePaymentResponse
AuthorizePaymentUseCase::Execute(
    const contracts::AuthorizePaymentRequest& request) {

    // Execute business logic through stateless domain service
    auto result = auth_service_->Authorize(request.amount, request.merchant_id);

    // Persist state through repository (repository assigns id)
    auto saved_payment = repository_->Save(
        request.amount,
        request.merchant_id,
        result.status,
        result.decline_reason);

    // Return contract model
    return contracts::AuthorizePaymentResponse{
        saved_payment.id,
        result.is_authorized,
        saved_payment.status,
        saved_payment.amount,
        saved_payment.decline_reason
    };
}

} // namespace logos::payment::service::core::application::use_cases
