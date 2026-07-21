#pragma once

#include <string>
#include <optional>

namespace mypaymentservice::rabbitmq_host::messages {

/// @brief Incoming message contract for payment authorization.
/// @details Mirrors the C# MassTransit AuthorizePaymentCommand record.
///          This is a transport-level message; it is mapped to an
///          Application Contract Model before invoking a Use Case.
///
/// @note Message contracts are NOT Domain types. They are owned by the
///       Host Unit and translated by the Mappings layer (AP-003).
struct AuthorizePaymentCommand {
    std::string message_id;                    // Unique message identifier (UUID)
    std::string amount;                        // Major units, e.g. "100.00"
    std::string currency;                      // Currency code, e.g. "USD"
    std::string merchant_id;                   // Merchant identifier
    std::optional<std::string> correlation_id; // Optional saga/correlation id
};

} // namespace mypaymentservice::rabbitmq_host::messages
