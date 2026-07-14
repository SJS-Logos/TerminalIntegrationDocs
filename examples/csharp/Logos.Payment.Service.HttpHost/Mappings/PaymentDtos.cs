namespace Logos.Payment.Service.HttpHost.Mappings;

public class AuthorizePaymentDto
{
    public string? PaymentId { get; set; }
    public bool IsAuthorized { get; set; }
    public string? Status { get; set; }
    public decimal Amount { get; set; }
    public string Currency { get; set; } = string.Empty;
    public string MerchantId { get; set; } = string.Empty;
    public string? DeclineReason { get; set; }
}

public class PaymentDetailsDto
{
    public string PaymentId { get; set; } = string.Empty;
    public decimal Amount { get; set; }
    public string Currency { get; set; } = string.Empty;
    public string MerchantId { get; set; } = string.Empty;
    public string Status { get; set; } = string.Empty;
    public DateTime CreatedAt { get; set; }
    public string? DeclineReason { get; set; }
}
