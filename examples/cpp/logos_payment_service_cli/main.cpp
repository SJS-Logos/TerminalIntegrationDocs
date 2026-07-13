#include "logos_payment_service_cli/cli_parser.h"
#include "logos_payment_service_cli/commands/authorize_command.h"
#include "logos_payment_service_cli/commands/get_payment_command.h"
#include "logos_payment_service_application/container/service_container.h"
#include "logos_payment_service_application/use_cases/authorize_payment_use_case.h"
#include "logos_payment_service_application/use_cases/get_payment_use_case.h"
#include "logos_payment_service_domain/services/payment_authorization_service.h"
#include "logos_payment_service_adapters/in_memory_payment_repository.h"
#include "logos_payment_service_adapters/simple_fraud_detection_service.h"
#include <iostream>
#include <memory>

using namespace logos::payment_service;

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
    cli::CliParser parser(argc, argv);

    if (!parser.IsValid() || parser.GetCommand() == "help") {
        PrintHelp();
        return 0;
    }

    // Create service container
    application::container::ServiceContainer container;

    // Register adapters (infrastructure layer)
    container.Register<domain::abstractions::IPaymentRepository>(
        std::make_unique<adapters::InMemoryPaymentRepository>());

    container.Register<domain::abstractions::IFraudDetectionService>(
        std::make_unique<adapters::SimpleFraudDetectionService>());

    // Register domain service with configuration
    auto maximum_amount = domain::value_objects::Money::FromCents(1000000, "USD");  // $10,000.00
    container.Register<domain::services::PaymentAuthorizationService>(
        std::make_unique<domain::services::PaymentAuthorizationService>(
            container.Resolve<domain::abstractions::IFraudDetectionService>(),
            maximum_amount));

    // Create use cases with resolved dependencies
    application::use_cases::AuthorizePaymentUseCase authorize_use_case(
        container.Resolve<domain::services::PaymentAuthorizationService>(),
        container.Resolve<domain::abstractions::IPaymentRepository>());

    application::use_cases::GetPaymentUseCase get_use_case(
        container.Resolve<domain::abstractions::IPaymentRepository>());

    // Route to command handlers
    std::string command = parser.GetCommand();

    if (command == "authorize") {
        cli::commands::AuthorizeCommand cmd(&authorize_use_case);
        return cmd.Execute(parser);
    } else if (command == "get") {
        cli::commands::GetPaymentCommand cmd(&get_use_case);
        return cmd.Execute(parser);
    } else {
        std::cerr << "Error: Unknown command: " << command << "\n\n";
        PrintHelp();
        return 1;
    }
}
