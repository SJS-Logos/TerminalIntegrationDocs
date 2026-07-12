using Logos.PaymentService.Application.UseCases;
using Logos.PaymentService.Domain.Abstractions;
using Logos.PaymentService.Domain.Services;
using Logos.PaymentService.Domain.ValueObjects;
using Logos.PaymentService.Adapters;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

builder.Services.AddSingleton<IPaymentRepository, InMemoryPaymentRepository>();
builder.Services.AddSingleton<IFraudDetectionService, SimpleFraudDetectionService>();

builder.Services.AddSingleton<PaymentAuthorizationService>(sp =>
{
    var fraudService = sp.GetRequiredService<IFraudDetectionService>();
    var maxAmount = new Money(10000m, "USD");
    return new PaymentAuthorizationService(fraudService, maxAmount);
});

builder.Services.AddScoped<AuthorizePaymentUseCase>();
builder.Services.AddScoped<GetPaymentUseCase>();

var app = builder.Build();

// Add a simple test endpoint
app.MapGet("/", () => "API is running! Swagger should be at /swagger/index.html");

app.UseSwagger();
app.UseSwaggerUI(c =>
{
    c.SwaggerEndpoint("/swagger/v1/swagger.json", "Payment Service API v1");
    c.RoutePrefix = "swagger";
});

app.UseAuthorization();
app.MapControllers();

app.Run();
