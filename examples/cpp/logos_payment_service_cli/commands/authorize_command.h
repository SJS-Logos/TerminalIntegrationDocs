#pragma once

#include "logos_payment_service_application/use_cases/authorize_payment_use_case.h"
#include "logos_payment_service_cli/cli_parser.h"

namespace logos::payment_service::cli::commands {

/// @brief CLI command for authorizing payments
/// @details Parses CLI arguments and invokes AuthorizePaymentUseCase
/// 
/// Usage:
///   payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001
/// 
/// @see application::use_cases::AuthorizePaymentUseCase
class AuthorizeCommand {
public:
    explicit AuthorizeCommand(
        application::use_cases::AuthorizePaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    static std::string PaymentStatusToString(domain::value_objects::PaymentStatus status);

    application::use_cases::AuthorizePaymentUseCase* use_case_;  // Non-owning
};

} // namespace logos::payment_service::cli::commands
