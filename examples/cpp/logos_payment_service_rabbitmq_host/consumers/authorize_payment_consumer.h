#pragma once

#include <string>

#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"

namespace logos::payment::service::rabbitmq_host::consumers {

/// @brief Transport endpoint (consumer) for payment authorization messages.
/// @details The C++ equivalent of the C# MassTransit AuthorizePaymentConsumer.
///          Receives a raw message payload from the broker, maps it to an
///          Application Contract, invokes the Use Case, and produces a
///          serialized PaymentAuthorizedEvent to publish.
///
/// Thread lifecycle: Deliver -> Map -> Invoke Use Case -> Map Result -> Publish -> Ack
///
/// @note This consumer contains NO business logic. All business decisions are
///       made in the Domain layer, invoked via the Use Case (AP-003).
///
/// @see core::application::use_cases::AuthorizePaymentUseCase
class AuthorizePaymentConsumer {
public:
    explicit AuthorizePaymentConsumer(
        core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case);

    /// @brief Handle a single delivered message.
    /// @param message_payload Raw JSON payload delivered by the broker.
    /// @return Serialized PaymentAuthorizedEvent JSON to be published.
    /// @throws std::runtime_error if the payload is invalid; the caller is
    ///         expected to nack/dead-letter the message (mirrors the C#
    ///         consumer rethrowing so MassTransit can retry).
    std::string Consume(const std::string& message_payload);

private:
    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case_;  // Non-owning
};

} // namespace logos::payment::service::rabbitmq_host::consumers
