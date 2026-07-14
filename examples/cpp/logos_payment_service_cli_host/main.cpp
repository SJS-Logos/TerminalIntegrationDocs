#include "logos_payment_service_cli_host/cli_parser.h"
#include "logos_payment_service_cli_host/commands/authorize_command.h"
#include "logos_payment_service_cli_host/commands/get_payment_command.h"
#include "logos_payment_service_core/application/container/service_container.h"
#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"
#include "logos_payment_service_core/application/use_cases/get_payment_use_case.h"
#include "logos_payment_service_core/domain/services/payment_authorization_service.h"
#include "logos_payment_service_infrastructure/in_memory_payment_repository.h"
#include "logos_payment_service_infrastructure/simple_fraud_detection_service.h"
#include <iostream>
#include <memory>

using namespace logos::payment::service;

void PrintHelp() {
    std::cout << "Payment Service CLI\n\n";
    std::cout << "Usage: payment-cli <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  authorize    Authorize a new payment\n";
    std::cout << "  get          Retrieve payment details by ID\n";
    std::cout << "  help         Show this help message\n\n";
    std::cout << "Use 'payment-cli <command> --help' for more information about a command.\n\n";
}

int main(int argc, char* argv[]) {
    cli_host::CliParser parser(argc, argv);

    if (!parser.IsValid() || parser.GetCommand() == "help") {
        PrintHelp();
        return 0;
    }

    // Create service container
    core::application::container::ServiceContainer container;

    // Register infrastructure (capability implementations)
    container.Register<core::capabilities::IPaymentRepository>(
        std::make_unique<infrastructure::InMemoryPaymentRepository>());

    container.Register<core::capabilities::IFraudDetectionService>(
        std::make_unique<infrastructure::SimpleFraudDetectionService>());

    // Register domain service with configuration
    auto maximum_amount = core::shared_kernel::Money::FromMinorUnits(1000000, "USD");  // $10,000.00
    container.Register<core::domain::services::PaymentAuthorizationService>(
        std::make_unique<core::domain::services::PaymentAuthorizationService>(
            container.Resolve<core::capabilities::IFraudDetectionService>(),
            maximum_amount));

    // Create use cases with resolved dependencies
    core::application::use_cases::AuthorizePaymentUseCase authorize_use_case(
        container.Resolve<core::domain::services::PaymentAuthorizationService>(),
        container.Resolve<core::capabilities::IPaymentRepository>());

    core::application::use_cases::GetPaymentUseCase get_use_case(
        container.Resolve<core::capabilities::IPaymentRepository>());

    // Route to command handlers
    std::string command = parser.GetCommand();

    if (command == "authorize") {
        cli_host::commands::AuthorizeCommand cmd(&authorize_use_case);
        return cmd.Execute(parser);
    } else if (command == "get") {
        cli_host::commands::GetPaymentCommand cmd(&get_use_case);
        return cmd.Execute(parser);
    } else {
        std::cerr << "Error: Unknown command: " << command << "\n\n";
        PrintHelp();
        return 1;
    }
}
