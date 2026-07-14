using PaymentEntity = Logos.Payment.Core.Domain.Entities.Payment;

namespace Logos.Payment.Core.Capabilities;

public interface IPaymentRepository
{
    void Save(PaymentEntity payment);
    PaymentEntity? GetById(string id);
}
