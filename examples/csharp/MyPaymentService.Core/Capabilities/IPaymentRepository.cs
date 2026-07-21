using PaymentEntity = MyPaymentService.Core.Domain.Entities.Payment;

namespace MyPaymentService.Core.Capabilities;

public interface IPaymentRepository
{
    void Save(PaymentEntity payment);
    PaymentEntity? GetById(string id);
}
