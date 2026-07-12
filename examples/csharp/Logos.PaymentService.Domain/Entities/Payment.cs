using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.Domain.Entities;

public class Payment
{
    public string Id { get; }
    public Money Amount { get; }
    public string MerchantId { get; }
    public PaymentStatus Status { get; private set; }
    public DateTime CreatedAt { get; }
    public string? DeclineReason { get; private set; }

    public Payment(string id, Money amount, string merchantId)
    {
        if (string.IsNullOrWhiteSpace(id))
            throw new ArgumentException("Id is required", nameof(id));
        if (string.IsNullOrWhiteSpace(merchantId))
            throw new ArgumentException("MerchantId is required", nameof(merchantId));

        Id = id;
        Amount = amount ?? throw new ArgumentNullException(nameof(amount));
        MerchantId = merchantId;
        Status = PaymentStatus.Pending;
        CreatedAt = DateTime.UtcNow;
    }

    public void Authorize()
    {
        Status = PaymentStatus.Authorized;
    }

    public void Decline(string reason)
    {
        Status = PaymentStatus.Declined;
        DeclineReason = reason;
    }
}
