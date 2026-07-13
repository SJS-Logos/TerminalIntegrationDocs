#pragma once

#include "logos_payment_service_application/use_cases/get_payment_use_case.h"
#include "logos_payment_service_cli/cli_parser.h"

namespace logos::payment_service::cli::commands {

/// @brief CLI command for retrieving payment details
/// @details Parses CLI arguments and invokes GetPaymentUseCase
/// 
/// Usage:
///   payment-cli get <payment-id>
/// 
/// @see application::use_cases::GetPaymentUseCase
class GetPaymentCommand {
public:
    explicit GetPaymentCommand(application::use_cases::GetPaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    static std::string PaymentStatusToString(domain::value_objects::PaymentStatus status);

    application::use_cases::GetPaymentUseCase* use_case_;  // Non-owning
};

} // namespace logos::payment_service::cli::commands
