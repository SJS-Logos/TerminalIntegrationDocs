using MyPaymentService.Core.SharedKernel;

namespace MyPaymentService.Core.Capabilities;

public interface IFraudDetectionService
{
    bool IsSuspicious(Money amount, string merchantId);
}
