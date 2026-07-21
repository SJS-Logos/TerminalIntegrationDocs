namespace MyPaymentService.MasstransitHost.Messages;

public record AuthorizePaymentCommand
{
    public Guid MessageId { get; init; }
    public decimal Amount { get; init; }
    public string Currency { get; init; } = string.Empty;
    public string MerchantId { get; init; } = string.Empty;
    public Guid? CorrelationId { get; init; }
}
