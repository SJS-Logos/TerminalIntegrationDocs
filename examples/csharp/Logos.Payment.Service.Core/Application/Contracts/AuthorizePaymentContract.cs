using Logos.Payment.Service.Core.SharedKernel;

namespace Logos.Payment.Service.Core.Application.Contracts;

/// <summary>
/// Request to authorize a payment transaction.
/// </summary>
/// <param name="Amount">The monetary amount to authorize, including currency.</param>
/// <param name="MerchantId">The unique identifier of the merchant initiating the payment.</param>
public record AuthorizePaymentRequest(
    Money Amount,
    string MerchantId);

/// <summary>
/// Response from a payment authorization request.
/// </summary>
/// <param name="PaymentId">The unique identifier assigned to this payment transaction.</param>
/// <param name="IsAuthorized">Indicates whether the payment was successfully authorized.</param>
/// <param name="Status">The current status of the payment.</param>
/// <param name="Amount">The authorized monetary amount.</param>
/// <param name="DeclineReason">The reason for decline if the payment was not authorized. Null if authorized.</param>
public record AuthorizePaymentResponse(
    string PaymentId,
    bool IsAuthorized,
    PaymentStatus Status,
    Money Amount,
    string? DeclineReason);
