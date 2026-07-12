using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.Domain.Abstractions;

public interface IFraudDetectionService
{
    bool IsSuspicious(Money amount, string merchantId);
}
