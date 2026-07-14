using Logos.Payment.Service.Core.Capabilities;
using PaymentEntity = Logos.Payment.Service.Core.Domain.Entities.Payment;

namespace Logos.Payment.Service.Infrastructure.InMemory;

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
