using Logos.Payment.Core.SharedKernel;

namespace Logos.Payment.Core.Application.Contracts;

public record AuthorizePaymentRequest(
    Money Amount,
    string MerchantId);

public record AuthorizePaymentResponse(
    string PaymentId,
    bool IsAuthorized,
    PaymentStatus Status,
    Money Amount,
    string? DeclineReason);
