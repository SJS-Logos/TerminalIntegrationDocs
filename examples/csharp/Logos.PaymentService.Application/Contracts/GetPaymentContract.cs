using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.Application.Contracts;

public record GetPaymentRequest(string PaymentId);

public record GetPaymentResponse(
    string PaymentId,
    Money Amount,
    string MerchantId,
    PaymentStatus Status,
    DateTime CreatedAt,
    string? DeclineReason);
