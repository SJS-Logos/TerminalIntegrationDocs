using Microsoft.AspNetCore.Mvc;
using Logos.Payment.Service.Core.Application.UseCases;
using Logos.Payment.Service.Core.Application.Contracts;
using Logos.Payment.Service.Core.SharedKernel;
using Logos.Payment.Service.HttpHost.Mappings;

namespace Logos.Payment.Service.HttpHost.Controllers;

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
