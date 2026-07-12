using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Entities;
using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.Domain.Services;

public class PaymentAuthorizationService(
    IFraudDetectionService fraudDetectionService,
    Money maximumAmount)
{
    public bool Authorize(Payment payment)
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
