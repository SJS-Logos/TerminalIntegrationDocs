# Build and Verify Script for C# Examples
# This script builds the example and verifies it compiles successfully

Write-Host "???  Building Logos Payment Service Example..." -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"

try {
    Push-Location "$PSScriptRoot"

    # Build the solution
    Write-Host "Building solution..." -ForegroundColor Yellow
    dotnet build --configuration Release

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "? Build successful!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Projects built:" -ForegroundColor Cyan
        Write-Host "  ? Logos.PaymentService.Domain" -ForegroundColor Green
        Write-Host "  ? Logos.PaymentService.Application" -ForegroundColor Green
        Write-Host "  ? Logos.PaymentService.Adapters" -ForegroundColor Green
        Write-Host "  ? Logos.PaymentService.WebApi" -ForegroundColor Green
        Write-Host "  ? Logos.PaymentService.Messaging" -ForegroundColor Green
        Write-Host ""
        Write-Host "To run the API:" -ForegroundColor Yellow
        Write-Host "  cd Logos.PaymentService.WebApi" -ForegroundColor White
        Write-Host "  dotnet run" -ForegroundColor White
        Write-Host ""
        Write-Host "Then navigate to: https://localhost:5001/swagger" -ForegroundColor White
        Write-Host ""
    } else {
        Write-Host ""
        Write-Host "? Build failed!" -ForegroundColor Red
        exit 1
    }
}
catch {
    Write-Host ""
    Write-Host "? Error: $_" -ForegroundColor Red
    exit 1
}
finally {
    Pop-Location
}
