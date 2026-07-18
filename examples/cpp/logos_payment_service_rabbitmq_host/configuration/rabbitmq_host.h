#pragma once

#include <memory>

#include "logos_payment_service_rabbitmq_host/configuration/rabbitmq_configuration.h"
#include "logos_payment_service_rabbitmq_host/consumers/authorize_payment_consumer.h"

namespace logos::payment::service::rabbitmq_host::configuration {

/// @brief Mechanical wiring that connects the broker to the consumer.
/// @details The C++ equivalent of the C# ServiceConfiguration.AddMassTransit
///          call. It:
///            - Opens a RabbitMQ connection/channel
///            - Declares the command queue and event exchange
///            - Subscribes the AuthorizePaymentConsumer to the queue
///            - Publishes the resulting PaymentAuthorizedEvent
///
/// This class contains NO business logic (AP-003). It is purely operational
/// infrastructure wiring.
///
/// @note This example uses AMQP-CPP (https://github.com/CopernicaMarketingSoftware/AMQP-CPP)
///       with a libev event loop. The RabbitMQ host target is only built when
///       these dependencies are available, so the default build remains
///       functional without them (see CMakeLists.txt).
class RabbitMqHost {
public:
    RabbitMqHost(RabbitMqConfiguration config,
                 consumers::AuthorizePaymentConsumer* consumer);
    ~RabbitMqHost();

    /// Connect, declare topology, and start consuming. Blocks until Stop()
    /// is called or the connection is lost. Returns false if it cannot start.
    bool Run();

    /// Request a graceful shutdown of the event loop.
    void Stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace logos::payment::service::rabbitmq_host::configuration
