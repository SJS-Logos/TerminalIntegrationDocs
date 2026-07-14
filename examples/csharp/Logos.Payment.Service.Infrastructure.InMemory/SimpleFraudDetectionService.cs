using Logos.Payment.Service.Core.Capabilities;
using Logos.Payment.Service.Core.SharedKernel;

namespace Logos.Payment.Service.Infrastructure.InMemory;

public class SimpleFraudDetectionService : IFraudDetectionService
{
    public bool IsSuspicious(Money amount, string merchantId)
    {
        return amount.GetAmount() > 5000m;
    }
}
