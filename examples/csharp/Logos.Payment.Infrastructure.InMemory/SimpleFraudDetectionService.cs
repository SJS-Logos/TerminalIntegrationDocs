using Logos.Payment.Core.Capabilities;
using Logos.Payment.Core.SharedKernel;

namespace Logos.Payment.Infrastructure.InMemory;

public class SimpleFraudDetectionService : IFraudDetectionService
{
    public bool IsSuspicious(Money amount, string merchantId)
    {
        return amount.GetAmount() > 5000m;
    }
}
