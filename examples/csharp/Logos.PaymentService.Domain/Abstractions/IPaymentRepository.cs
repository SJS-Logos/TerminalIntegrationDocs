using Logos.PaymentService.Domain.Entities;

namespace Logos.PaymentService.Domain.Abstractions;

public interface IPaymentRepository
{
    void Save(Payment payment);
    Payment? GetById(string id);
}
