#include "logos_payment_cli_host/commands/authorize_command.h"
#include "logos_payment_core/shared_kernel/money.h"
#include <iostream>
#include <iomanip>

namespace logos::payment::cli_host::commands {

AuthorizeCommand::AuthorizeCommand(
    core::application::use_cases::AuthorizePaymentUseCase* use_case)
    : use_case_(use_case) {
}

int AuthorizeCommand::Execute(const CliParser& parser) {
    if (parser.HasFlag("help")) {
        PrintUsage();
        return 0;
    }

    // Parse CLI arguments
    std::string amount_str = parser.GetOption("amount");
    std::string currency = parser.GetOption("currency");
    std::string merchant_id = parser.GetOption("merchant");

    // Validate required arguments
    if (amount_str.empty() || currency.empty() || merchant_id.empty()) {
        std::cerr << "Error: Missing required arguments\n\n";
        PrintUsage();
        return 1;
    }

    // Map CLI arguments to Contract Model
    try {
        core::application::contracts::AuthorizePaymentRequest request{
            core::shared_kernel::Money(amount_str, currency),
            merchant_id
        };

        // Execute use case (synchronous)
        auto result = use_case_->Execute(request);

        // Output result to console
        std::cout << "\n";
        std::cout << "=== Payment Authorization Result ===\n";
        std::cout << "Payment ID:     " << result.payment_id << "\n";
        std::cout << "Authorized:     " << (result.is_authorized ? "YES" : "NO") << "\n";
        std::cout << "Status:         " << PaymentStatusToString(result.status) << "\n";
        std::cout << "Amount:         " << result.amount.ToString() << "\n";

        if (result.decline_reason) {
            std::cout << "Decline Reason: " << *result.decline_reason << "\n";
        }

        std::cout << "====================================\n\n";

        return result.is_authorized ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

void AuthorizeCommand::PrintUsage() {
    std::cout << "Usage: payment-cli authorize [OPTIONS]\n\n";
    std::cout << "Authorize a payment\n\n";
    std::cout << "Options:\n";
    std::cout << "  --amount AMOUNT       Payment amount (required)\n";
    std::cout << "  --currency CURRENCY   Currency code, e.g., USD (required)\n";
    std::cout << "  --merchant MERCHANT   Merchant identifier (required)\n";
    std::cout << "  --help                Show this help message\n\n";
    std::cout << "Example:\n";
    std::cout << "  payment-cli authorize --amount 100.00 --currency USD --merchant MERCH-001\n\n";
}

std::string AuthorizeCommand::PaymentStatusToString(
    core::shared_kernel::PaymentStatus status) {
    switch (status) {
        case core::shared_kernel::PaymentStatus::Pending:
            return "Pending";
        case core::shared_kernel::PaymentStatus::Authorized:
            return "Authorized";
        case core::shared_kernel::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment::cli_host::commands
