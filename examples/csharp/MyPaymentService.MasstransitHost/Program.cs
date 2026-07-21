using MyPaymentService.MasstransitHost.Configuration;

var builder = Host.CreateApplicationBuilder(args);

builder.Services.AddPaymentServices(builder.Configuration);

var host = builder.Build();
host.Run();
