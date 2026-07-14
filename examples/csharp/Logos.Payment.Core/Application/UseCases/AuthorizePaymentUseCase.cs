using Logos.Payment.Core.Application.Contracts;
using Logos.Payment.Core.Capabilities;
using Logos.Payment.Core.Domain.Services;
using PaymentEntity = Logos.Payment.Core.Domain.Entities.Payment;

namespace Logos.Payment.Core.Application.UseCases;

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
