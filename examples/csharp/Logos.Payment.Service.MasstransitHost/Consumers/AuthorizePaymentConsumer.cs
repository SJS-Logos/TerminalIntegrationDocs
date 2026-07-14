using MassTransit;
using Microsoft.Extensions.Logging;
using Logos.Payment.Service.Core.Application.UseCases;
using Logos.Payment.Service.Core.Application.Contracts;
using Logos.Payment.Service.Core.SharedKernel;
using Logos.Payment.Service.MasstransitHost.Messages;

namespace Logos.Payment.Service.MasstransitHost.Consumers;

public class AuthorizePaymentConsumer(
    AuthorizePaymentUseCase authorizePaymentUseCase,
    ILogger<AuthorizePaymentConsumer> logger) : IConsumer<AuthorizePaymentCommand>
{
    public async Task Consume(ConsumeContext<AuthorizePaymentCommand> context)
    {
        var message = context.Message;

        logger.LogInformation(
            "Received AuthorizePaymentCommand: MessageId={MessageId}, Amount={Amount} {Currency}",
            message.MessageId, message.Amount, message.Currency);

        try
        {
            var request = new AuthorizePaymentRequest(
                Amount: new Money(message.Amount, message.Currency),
                MerchantId: message.MerchantId);

            var result = authorizePaymentUseCase.Execute(request);

            await context.Publish(new PaymentAuthorizedEvent
            {
                PaymentId = result.PaymentId,
                IsAuthorized = result.IsAuthorized,
                Status = result.Status.ToString(),
                Amount = result.Amount.GetAmount(),
                Currency = result.Amount.GetCurrency(),
                DeclineReason = result.DeclineReason,
                AuthorizedAt = DateTime.UtcNow,
                CorrelationId = message.CorrelationId
            });

            logger.LogInformation(
                "Payment authorization completed: PaymentId={PaymentId}, IsAuthorized={IsAuthorized}",
                result.PaymentId, result.IsAuthorized);
        }
        catch (Exception ex)
        {
            logger.LogError(ex,
                "Failed to process AuthorizePaymentCommand: MessageId={MessageId}",
                message.MessageId);

            throw;
        }
    }
}
