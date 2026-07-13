#include "logos_payment_service_application/use_cases/get_payment_use_case.h"

namespace logos::payment_service::application::use_cases {

GetPaymentUseCase::GetPaymentUseCase(domain::abstractions::IPaymentRepository* repository)
    : repository_(repository) {
}

std::optional<contracts::GetPaymentResponse> 
GetPaymentUseCase::Execute(const std::string& payment_id) {
    return repository_->GetById(payment_id);
}

} // namespace logos::payment_service::application::use_cases
