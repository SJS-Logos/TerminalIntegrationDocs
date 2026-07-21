#pragma once

#include "mypaymentservice_core/application/use_cases/authorize_payment_use_case.h"
#include "mypaymentservice_cli_host/cli_parser.h"

namespace mypaymentservice::cli_host::commands {

/// @brief CLI command (transport endpoint) for authorizing payments
/// @details Parses CLI arguments and invokes AuthorizePaymentUseCase
///
/// Usage:
///   payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001
class AuthorizeCommand {
public:
    explicit AuthorizeCommand(
        core::application::use_cases::AuthorizePaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    static std::string PaymentStatusToString(core::shared_kernel::PaymentStatus status);

    core::application::use_cases::AuthorizePaymentUseCase* use_case_;  // Non-owning
};

} // namespace mypaymentservice::cli_host::commands
