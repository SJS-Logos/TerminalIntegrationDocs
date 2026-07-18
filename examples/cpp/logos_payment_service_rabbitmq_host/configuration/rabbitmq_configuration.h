#pragma once

#include <string>
#include <cstdint>
#include <cstdlib>

namespace logos::payment::service::rabbitmq_host::configuration {

/// @brief Mechanical wiring: RabbitMQ connection settings.
/// @details Contains NO business logic (AP-003) - only operational
///          configuration. Values default to a local broker and can be
///          overridden via environment variables, mirroring the C#
///          appsettings "RabbitMQ" section.
struct RabbitMqConfiguration {
    std::string host = "localhost";
    uint16_t port = 5672;
    std::string virtual_host = "/";
    std::string username = "guest";
    std::string password = "guest";

    /// Queue the consumer subscribes to (incoming commands).
    std::string command_queue = "authorize-payment-command";

    /// Exchange used to publish outgoing events.
    std::string event_exchange = "payment-authorized-event";

    /// Load configuration from environment variables, falling back to defaults.
    /// Recognized variables:
    ///   RABBITMQ_HOST, RABBITMQ_PORT, RABBITMQ_VHOST,
    ///   RABBITMQ_USERNAME, RABBITMQ_PASSWORD,
    ///   RABBITMQ_COMMAND_QUEUE, RABBITMQ_EVENT_EXCHANGE
    static RabbitMqConfiguration FromEnvironment() {
        RabbitMqConfiguration config;

        if (const char* v = std::getenv("RABBITMQ_HOST")) config.host = v;
        if (const char* v = std::getenv("RABBITMQ_PORT"))
            config.port = static_cast<uint16_t>(std::atoi(v));
        if (const char* v = std::getenv("RABBITMQ_VHOST")) config.virtual_host = v;
        if (const char* v = std::getenv("RABBITMQ_USERNAME")) config.username = v;
        if (const char* v = std::getenv("RABBITMQ_PASSWORD")) config.password = v;
        if (const char* v = std::getenv("RABBITMQ_COMMAND_QUEUE")) config.command_queue = v;
        if (const char* v = std::getenv("RABBITMQ_EVENT_EXCHANGE")) config.event_exchange = v;

        return config;
    }
};

} // namespace logos::payment::service::rabbitmq_host::configuration
