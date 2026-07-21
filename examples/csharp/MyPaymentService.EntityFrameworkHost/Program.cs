using MyPaymentService.Core.Application.Contracts;
using MyPaymentService.Core.Application.UseCases;
using MyPaymentService.Core.SharedKernel;
using MyPaymentService.EntityFrameworkHost.Configuration;

var builder = WebApplication.CreateBuilder(args);

var connectionString =
    builder.Configuration.GetConnectionString("Payments") ?? "Data Source=payments.db";

builder.Services.AddPaymentServices(connectionString);

var app = builder.Build();

// Schema readiness precondition (AP-007 §8.2): migration runs before the host
// serves traffic. An unhandled failure here aborts startup, so the host never
// serves requests against an unmigrated store.
await app.Services.InitializeInfrastructureAsync();

app.MapPost("/api/payments/authorize", (AuthorizePaymentDto dto, AuthorizePaymentUseCase useCase) =>
{
    var response = useCase.Execute(
        new AuthorizePaymentRequest(new Money(dto.Amount, dto.Currency), dto.MerchantId));
    return Results.Ok(response);
});

app.MapGet("/api/payments/{id}", (string id, GetPaymentUseCase useCase) =>
{
    var response = useCase.Execute(new GetPaymentRequest(id));
    return response is null ? Results.NotFound() : Results.Ok(response);
});

app.Run();

internal record AuthorizePaymentDto(decimal Amount, string Currency, string MerchantId);
