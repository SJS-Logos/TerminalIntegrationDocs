namespace MyPaymentService.MasstransitHost.Messages;

public record PaymentAuthorizedEvent
{
    public string PaymentId { get; init; } = string.Empty;
    public bool IsAuthorized { get; init; }
    public string Status { get; init; } = string.Empty;
    public decimal Amount { get; init; }
    public string Currency { get; init; } = string.Empty;
    public string? DeclineReason { get; init; }
    public DateTime AuthorizedAt { get; init; }
    public Guid? CorrelationId { get; init; }
}
