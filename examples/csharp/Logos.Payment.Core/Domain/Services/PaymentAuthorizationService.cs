using Logos.Payment.Core.Capabilities;
using Logos.Payment.Core.SharedKernel;
using PaymentEntity = Logos.Payment.Core.Domain.Entities.Payment;

namespace Logos.Payment.Core.Domain.Services;

public class PaymentAuthorizationService(
    IFraudDetectionService fraudDetectionService,
    Money maximumAmount)
{
    public bool Authorize(PaymentEntity payment)
    {
        if (payment.Amount.GetAmount() > maximumAmount.GetAmount())
        {
            payment.Decline("Amount exceeds maximum");
            return false;
        }

        if (fraudDetectionService.IsSuspicious(payment.Amount, payment.MerchantId))
        {
            payment.Decline("Flagged by fraud detection");
            return false;
        }

        payment.Authorize();
        return true;
    }
}
