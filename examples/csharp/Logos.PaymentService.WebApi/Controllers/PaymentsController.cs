using Microsoft.AspNetCore.Mvc;
using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Application.Contracts;
using Logos.PaymentService.Domain.ValueObjects;

namespace Logos.PaymentService.WebApi.Controllers;

[ApiController]
[Route("api/[controller]")]
public class PaymentsController(
    AuthorizePaymentUseCase authorizePaymentUseCase,
    GetPaymentUseCase getPaymentUseCase) : ControllerBase
{
    [HttpPost("authorize")]
    [ProducesResponseType(typeof(AuthorizePaymentDto), StatusCodes.Status200OK)]
    [ProducesResponseType(StatusCodes.Status400BadRequest)]
    public IActionResult AuthorizePayment([FromBody] AuthorizePaymentDto request)
    {
        var useCaseRequest = new AuthorizePaymentRequest(
            Amount: new Money(request.Amount, request.Currency),
            MerchantId: request.MerchantId);

        var result = authorizePaymentUseCase.Execute(useCaseRequest);

        var response = new AuthorizePaymentDto
        {
            PaymentId = result.PaymentId,
            IsAuthorized = result.IsAuthorized,
            Status = result.Status.ToString(),
            Amount = result.Amount.GetAmount(),
            Currency = result.Amount.GetCurrency(),
            DeclineReason = result.DeclineReason
        };

        return Ok(response);
    }

    [HttpGet("{id}")]
    [ProducesResponseType(typeof(PaymentDetailsDto), StatusCodes.Status200OK)]
    [ProducesResponseType(StatusCodes.Status404NotFound)]
    public IActionResult GetPayment(string id)
    {
        var request = new GetPaymentRequest(id);
        var result = getPaymentUseCase.Execute(request);

        if (result == null)
            return NotFound();

        var response = new PaymentDetailsDto
        {
            PaymentId = result.PaymentId,
            Amount = result.Amount.GetAmount(),
            Currency = result.Amount.GetCurrency(),
            MerchantId = result.MerchantId,
            Status = result.Status.ToString(),
            CreatedAt = result.CreatedAt,
            DeclineReason = result.DeclineReason
        };

        return Ok(response);
    }
}

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
