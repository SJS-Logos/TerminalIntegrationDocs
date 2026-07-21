#pragma once

#include <string>
#include <optional>

namespace mypaymentservice::rabbitmq_host::messages {

/// @brief Outgoing message contract published after authorization.
/// @details Mirrors the C# MassTransit PaymentAuthorizedEvent record.
///          Published to the broker so that other services can react to
///          the outcome of a payment authorization.
///
/// @note Like the incoming command, this is a transport-level message
///       owned by the Host Unit, not a Domain type.
struct PaymentAuthorizedEvent {
    std::string payment_id;                    // Assigned payment identifier
    bool is_authorized = false;                // Authorization outcome
    std::string status;                        // "Authorized", "Declined", ...
    std::string amount;                        // Major units, e.g. "100.00"
    std::string currency;                      // Currency code, e.g. "USD"
    std::optional<std::string> decline_reason; // Present when declined
    std::string authorized_at;                 // ISO 8601 UTC timestamp
    std::optional<std::string> correlation_id; // Echoed correlation id
};

} // namespace mypaymentservice::rabbitmq_host::messages
