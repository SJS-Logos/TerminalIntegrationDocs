using Logos.Payment.Core.SharedKernel;

namespace Logos.Payment.Core.Capabilities;

public interface IFraudDetectionService
{
    bool IsSuspicious(Money amount, string merchantId);
}
