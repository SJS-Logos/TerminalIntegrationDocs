using Logos.PaymentService.Application.Contracts;
using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Entities;
using Logos.PaymentService.Domain.Services;

namespace Logos.PaymentService.Application.UseCases;

public class AuthorizePaymentUseCase(
    PaymentAuthorizationService authorizationService,
    IPaymentRepository paymentRepository)
{
    public AuthorizePaymentResponse Execute(AuthorizePaymentRequest request)
    {
        var payment = new Payment(
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
