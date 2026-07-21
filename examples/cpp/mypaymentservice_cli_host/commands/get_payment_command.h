#pragma once

#include "mypaymentservice_core/application/use_cases/get_payment_use_case.h"
#include "mypaymentservice_cli_host/cli_parser.h"

namespace mypaymentservice::cli_host::commands {

/// @brief CLI command (transport endpoint) for retrieving payment details
/// @details Parses CLI arguments and invokes GetPaymentUseCase
///
/// Usage:
///   payment-cli get <payment-id>
class GetPaymentCommand {
public:
    explicit GetPaymentCommand(core::application::use_cases::GetPaymentUseCase* use_case);

    /// Execute the command with parsed CLI arguments
    int Execute(const CliParser& parser);

    /// Print usage information
    static void PrintUsage();

private:
    static std::string PaymentStatusToString(core::shared_kernel::PaymentStatus status);

    core::application::use_cases::GetPaymentUseCase* use_case_;  // Non-owning
};

} // namespace mypaymentservice::cli_host::commands
