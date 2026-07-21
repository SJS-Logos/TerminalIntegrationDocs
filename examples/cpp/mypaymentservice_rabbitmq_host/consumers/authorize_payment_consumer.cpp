#include "mypaymentservice_rabbitmq_host/consumers/authorize_payment_consumer.h"

#include "mypaymentservice_rabbitmq_host/mappings/message_mapping.h"

#include <iostream>
#include <stdexcept>

namespace mypaymentservice::rabbitmq_host::consumers {

AuthorizePaymentConsumer::AuthorizePaymentConsumer(
    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case)
    : authorize_use_case_(authorize_use_case) {
}

std::string AuthorizePaymentConsumer::Consume(const std::string& message_payload) {
    // Deserialize transport message.
    auto command_opt = mappings::MessageMapping::ParseAuthorizeCommand(message_payload);
    if (!command_opt) {
        // Invalid message: rethrow so the caller can dead-letter/retry.
        // Mirrors the C# consumer rethrowing on failure.
        throw std::runtime_error("Invalid AuthorizePaymentCommand payload");
    }
    const auto& command = *command_opt;

    std::cout << "Received AuthorizePaymentCommand: MessageId=" << command.message_id
              << ", Amount=" << command.amount << " " << command.currency << "\n";

    try {
        // Map transport message -> Contract Model.
        auto request = mappings::MessageMapping::ToContract(command);

        // Execute use case (synchronous). All business decisions happen here.
        auto result = authorize_use_case_->Execute(request);

        // Map result -> outgoing event and serialize for publishing.
        auto event = mappings::MessageMapping::ToEvent(result, command.correlation_id);

        std::cout << "Payment authorization completed: PaymentId=" << event.payment_id
                  << ", IsAuthorized=" << (event.is_authorized ? "true" : "false") << "\n";

        return mappings::MessageMapping::SerializeEvent(event);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to process AuthorizePaymentCommand: MessageId="
                  << command.message_id << ", Error=" << ex.what() << "\n";
        // Rethrow so the broker layer can nack/dead-letter (retry semantics).
        throw;
    }
}

} // namespace mypaymentservice::rabbitmq_host::consumers
