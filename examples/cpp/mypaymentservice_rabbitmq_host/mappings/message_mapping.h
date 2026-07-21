#pragma once

#include <string>
#include <optional>

#include "mypaymentservice_rabbitmq_host/messages/authorize_payment_command.h"
#include "mypaymentservice_rabbitmq_host/messages/payment_authorized_event.h"
#include "mypaymentservice_core/application/contracts/authorize_payment_request.h"
#include "mypaymentservice_core/application/contracts/authorize_payment_response.h"

namespace mypaymentservice::rabbitmq_host::mappings {

/// @brief Purely translational mapping between transport messages, JSON,
///        and Application Contract Models.
/// @details Contains NO business logic (AP-003). It only serializes,
///          deserializes, and maps field-by-field.
class MessageMapping {
public:
    /// Parse an incoming JSON payload into an AuthorizePaymentCommand.
    /// @return std::nullopt if the JSON is invalid or required fields are missing.
    static std::optional<messages::AuthorizePaymentCommand>
    ParseAuthorizeCommand(const std::string& json_payload);

    /// Map a transport command to the Application Contract Model.
    static core::application::contracts::AuthorizePaymentRequest
    ToContract(const messages::AuthorizePaymentCommand& command);

    /// Map an Application Contract result to the outgoing event message.
    static messages::PaymentAuthorizedEvent
    ToEvent(const core::application::contracts::AuthorizePaymentResponse& result,
            const std::optional<std::string>& correlation_id);

    /// Serialize an outgoing event to a JSON payload.
    static std::string SerializeEvent(const messages::PaymentAuthorizedEvent& event);
};

} // namespace mypaymentservice::rabbitmq_host::mappings
