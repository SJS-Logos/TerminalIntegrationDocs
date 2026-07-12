using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Entities;

namespace Logos.PaymentService.Adapters;

public class InMemoryPaymentRepository : IPaymentRepository
{
    private readonly Dictionary<string, Payment> _payments = new();

    public void Save(Payment payment)
    {
        _payments[payment.Id] = payment;
    }

    public Payment? GetById(string id)
    {
        _payments.TryGetValue(id, out var payment);
        return payment;
    }
}
