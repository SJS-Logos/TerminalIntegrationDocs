using MyPaymentService.Core.Capabilities;
using MyPaymentService.Core.SharedKernel;
using Microsoft.EntityFrameworkCore;
using PaymentEntity = MyPaymentService.Core.Domain.Entities.Payment;

namespace MyPaymentService.Infrastructure.EntityFramework;

/// <summary>
/// Entity Framework realization of the <see cref="IPaymentRepository"/> Capability.
/// Its sole responsibility is translation between domain scope and SQL scope
/// (AP-007 §4.1); it contains no business decisions.
/// </summary>
public class EntityFrameworkPaymentRepository : IPaymentRepository
{
    private readonly PaymentDbContext _context;

    public EntityFrameworkPaymentRepository(PaymentDbContext context)
    {
        _context = context;
    }

    public void Save(PaymentEntity payment)
    {
        var row = _context.Payments.Find(payment.Id);
        if (row is null)
        {
            _context.Payments.Add(ToRow(payment));
        }
        else
        {
            UpdateRow(row, payment);
        }

        _context.SaveChanges();
    }

    public PaymentEntity? GetById(string id)
    {
        var row = _context.Payments.AsNoTracking().FirstOrDefault(p => p.Id == id);
        return row is null ? null : ToDomain(row);
    }

    private static PaymentRow ToRow(PaymentEntity payment) => new()
    {
        Id = payment.Id,
        Amount = payment.Amount.GetAmount(),
        Currency = payment.Amount.GetCurrency(),
        MerchantId = payment.MerchantId,
        Status = payment.Status.ToString(),
        CreatedAt = payment.CreatedAt,
        DeclineReason = payment.DeclineReason
    };

    private static void UpdateRow(PaymentRow row, PaymentEntity payment)
    {
        row.Amount = payment.Amount.GetAmount();
        row.Currency = payment.Amount.GetCurrency();
        row.MerchantId = payment.MerchantId;
        row.Status = payment.Status.ToString();
        row.DeclineReason = payment.DeclineReason;
    }

    private static PaymentEntity ToDomain(PaymentRow row)
    {
        // Rebuild the domain entity through its public API. Status is restored
        // via the domain transition methods so no persistence-only setter leaks
        // into the Domain.
        var payment = new PaymentEntity(
            row.Id,
            new Money(row.Amount, row.Currency),
            row.MerchantId);

        switch (Enum.Parse<PaymentStatus>(row.Status))
        {
            case PaymentStatus.Authorized:
                payment.Authorize();
                break;
            case PaymentStatus.Declined:
                payment.Decline(row.DeclineReason ?? string.Empty);
                break;
        }

        return payment;
    }
}
