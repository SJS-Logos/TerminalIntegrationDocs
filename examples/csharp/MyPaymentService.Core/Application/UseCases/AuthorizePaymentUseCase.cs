using MyPaymentService.Core.Application.Contracts;
using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.Domain.Services;
using PaymentEntity = MyPaymentService.Core.Domain.Entities.Payment;

namespace MyPaymentService.Core.Application.UseCases;

public class AuthorizePaymentUseCase(
    PaymentAuthorizationService authorizationService,
    IPaymentRepository paymentRepository)
{
    public AuthorizePaymentResponse Execute(AuthorizePaymentRequest request)
    {
        var payment = new PaymentEntity(
            Guid.NewGuid().ToString(),
            request.Amount,
            request.MerchantId);

        var isAuthorized = authorizationService.Authorize(payment);

        paymentRepository.Save(payment);

        return new AuthorizePaymentResponse(
            payment.Id,
            isAuthorized,
            payment.Status,
            payment.Amount,
            payment.DeclineReason);
    }
}
