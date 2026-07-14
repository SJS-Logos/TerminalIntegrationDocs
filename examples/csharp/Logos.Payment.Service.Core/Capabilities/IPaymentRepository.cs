using PaymentEntity = Logos.Payment.Service.Core.Domain.Entities.Payment;

namespace Logos.Payment.Service.Core.Capabilities;

public interface IPaymentRepository
{
    void Save(PaymentEntity payment);
    PaymentEntity? GetById(string id);
}
