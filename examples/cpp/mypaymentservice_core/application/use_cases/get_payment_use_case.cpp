#include "mypaymentservice_core/application/use_cases/get_payment_use_case.h"

namespace mypaymentservice::core::application::use_cases {

GetPaymentUseCase::GetPaymentUseCase(capabilities::IPaymentRepository* repository)
    : repository_(repository) {
}

std::optional<contracts::GetPaymentResponse>
GetPaymentUseCase::Execute(const std::string& payment_id) {
    return repository_->GetById(payment_id);
}

} // namespace mypaymentservice::core::application::use_cases
