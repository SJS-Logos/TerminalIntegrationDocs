#include "logos_payment_service_cli/commands/get_payment_command.h"
#include <iostream>
#include <iomanip>
#include <ctime>

namespace logos::payment_service::cli::commands {

GetPaymentCommand::GetPaymentCommand(
    application::use_cases::GetPaymentUseCase* use_case)
    : use_case_(use_case) {
}

int GetPaymentCommand::Execute(const CliParser& parser) {
    if (parser.HasFlag("help")) {
        PrintUsage();
        return 0;
    }

    // Get payment ID from positional argument
    std::string payment_id = parser.GetPositional(0);

    if (payment_id.empty()) {
        std::cerr << "Error: Missing payment ID\n\n";
        PrintUsage();
        return 1;
    }

    // Execute use case (synchronous)
    auto result = use_case_->Execute(payment_id);

    if (!result) {
        std::cerr << "Error: Payment not found: " << payment_id << "\n";
        return 1;
    }

    // Output result to console
    std::cout << "\n";
    std::cout << "=== Payment Details ===\n";
    std::cout << "Payment ID:     " << result->id << "\n";
    std::cout << "Amount:         " << result->amount.ToString() << "\n";
    std::cout << "Merchant ID:    " << result->merchant_id << "\n";
    std::cout << "Status:         " << PaymentStatusToString(result->status) << "\n";

    // Format timestamp
    auto time_t = std::chrono::system_clock::to_time_t(result->created_at);
    std::cout << "Created At:     " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";

    if (result->decline_reason) {
        std::cout << "Decline Reason: " << *result->decline_reason << "\n";
    }

    std::cout << "=======================\n\n";

    return 0;
}

void GetPaymentCommand::PrintUsage() {
    std::cout << "Usage: payment-cli get <payment-id> [OPTIONS]\n\n";
    std::cout << "Retrieve payment details by ID\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  payment-id            Payment identifier (required)\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help                Show this help message\n\n";
    std::cout << "Example:\n";
    std::cout << "  payment-cli get PAY-000001\n\n";
}

std::string GetPaymentCommand::PaymentStatusToString(
    domain::value_objects::PaymentStatus status) {
    switch (status) {
        case domain::value_objects::PaymentStatus::Pending:
            return "Pending";
        case domain::value_objects::PaymentStatus::Authorized:
            return "Authorized";
        case domain::value_objects::PaymentStatus::Declined:
            return "Declined";
        default:
            return "Unknown";
    }
}

} // namespace logos::payment_service::cli::commands
