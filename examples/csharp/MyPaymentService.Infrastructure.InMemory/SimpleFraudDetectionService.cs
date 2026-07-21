using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.SharedKernel;

namespace MyPaymentService.Infrastructure.InMemory;

public class SimpleFraudDetectionService : IFraudDetectionService
{
    public bool IsSuspicious(Money amount, string merchantId)
    {
        return amount.GetAmount() > 5000m;
    }
}
