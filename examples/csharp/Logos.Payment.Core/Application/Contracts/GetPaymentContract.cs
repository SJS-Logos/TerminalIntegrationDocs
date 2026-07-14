using Logos.Payment.Core.SharedKernel;

namespace Logos.Payment.Core.Application.Contracts;

/// <summary>
/// Request to retrieve details of an existing payment transaction.
/// </summary>
/// <param name="PaymentId">The unique identifier of the payment to retrieve.</param>
public record GetPaymentRequest(
    string PaymentId);

/// <summary>
/// Response containing the details of a payment transaction.
/// </summary>
/// <param name="PaymentId">The unique identifier of the payment.</param>
/// <param name="Amount">The monetary amount of the payment, including currency.</param>
/// <param name="MerchantId">The unique identifier of the merchant who initiated the payment.</param>
/// <param name="Status">The current status of the payment.</param>
/// <param name="CreatedAt">The date and time when the payment was created.</param>
/// <param name="DeclineReason">The reason for decline if the payment was not authorized. Null if authorized.</param>
public record GetPaymentResponse(
    string PaymentId,
    Money Amount,
    string MerchantId,
    PaymentStatus Status,
    DateTime CreatedAt,
    string? DeclineReason);
