using MyPaymentService.Core.Capabilities;
using PaymentEntity = MyPaymentService.Core.Domain.Entities.Payment;

namespace MyPaymentService.Infrastructure.InMemory;

public class InMemoryPaymentRepository : IPaymentRepository
{
    private readonly Dictionary<string, PaymentEntity> _payments = new();

    public void Save(PaymentEntity payment)
    {
        _payments[payment.Id] = payment;
    }

    public PaymentEntity? GetById(string id)
    {
        _payments.TryGetValue(id, out var payment);
        return payment;
    }
}
