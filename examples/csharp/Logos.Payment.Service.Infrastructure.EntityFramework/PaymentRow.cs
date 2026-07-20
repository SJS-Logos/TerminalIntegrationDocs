namespace Logos.Payment.Service.Infrastructure.EntityFramework;

/// <summary>
/// Technology-scope persistence model (SQL row). Separate from the domain
/// <c>Payment</c> entity; the Adapter translates between them (AP-007 §4.1).
/// </summary>
public class PaymentRow
{
    public string Id { get; set; } = string.Empty;
    public decimal Amount { get; set; }
    public string Currency { get; set; } = string.Empty;
    public string MerchantId { get; set; } = string.Empty;
    public string Status { get; set; } = string.Empty;
    public DateTime CreatedAt { get; set; }
    public string? DeclineReason { get; set; }
}
