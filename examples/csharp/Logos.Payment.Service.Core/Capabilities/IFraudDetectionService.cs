using Logos.Payment.Service.Core.SharedKernel;

namespace Logos.Payment.Service.Core.Capabilities;

public interface IFraudDetectionService
{
    bool IsSuspicious(Money amount, string merchantId);
}
