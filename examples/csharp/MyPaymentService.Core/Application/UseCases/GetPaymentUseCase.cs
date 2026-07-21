using MyPaymentService.Core.Application.Contracts;
using MyPaymentService.Core.Capabilities;

namespace MyPaymentService.Core.Application.UseCases;

public class GetPaymentUseCase(IPaymentRepository paymentRepository)
{
    public GetPaymentResponse? Execute(GetPaymentRequest request)
    {
        var payment = paymentRepository.GetById(request.PaymentId);

        if (payment == null)
            return null;

        return new GetPaymentResponse(
            payment.Id,
            payment.Amount,
            payment.MerchantId,
            payment.Status,
            payment.CreatedAt,
            payment.DeclineReason);
    }
}
