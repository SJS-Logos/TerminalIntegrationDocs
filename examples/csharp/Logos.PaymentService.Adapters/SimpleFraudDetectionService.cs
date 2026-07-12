using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.Adapters;

public class SimpleFraudDetectionService : IFraudDetectionService
{
    public bool IsSuspicious(Money amount, string merchantId)
    {
        return amount.GetAmount() > 5000m;
    }
}
