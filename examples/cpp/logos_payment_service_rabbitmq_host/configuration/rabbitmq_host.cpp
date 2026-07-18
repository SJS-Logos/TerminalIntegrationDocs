#include "logos_payment_service_rabbitmq_host/configuration/rabbitmq_host.h"

#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ev.h>

#include <iostream>
#include <string>

namespace logos::payment::service::rabbitmq_host::configuration {

/// @brief Private implementation holding the libev loop and AMQP objects.
/// @details Kept out of the header (pimpl) so that the AMQP-CPP and libev
///          headers do not leak into the rest of the codebase.
struct RabbitMqHost::Impl {
    RabbitMqConfiguration config;
    consumers::AuthorizePaymentConsumer* consumer;  // Non-owning
    struct ev_loop* loop = nullptr;

    Impl(RabbitMqConfiguration cfg, consumers::AuthorizePaymentConsumer* c)
        : config(std::move(cfg)), consumer(c) {}
};

RabbitMqHost::RabbitMqHost(RabbitMqConfiguration config,
                           consumers::AuthorizePaymentConsumer* consumer)
    : impl_(std::make_unique<Impl>(std::move(config), consumer)) {
}

RabbitMqHost::~RabbitMqHost() = default;

bool RabbitMqHost::Run() {
    impl_->loop = EV_DEFAULT;

    // Build the AMQP connection address (mechanical wiring only).
    AMQP::Address address(
        impl_->config.host,
        impl_->config.port,
        AMQP::Login(impl_->config.username, impl_->config.password),
        impl_->config.virtual_host);

    AMQP::LibEvHandler handler(impl_->loop);
    AMQP::TcpConnection connection(&handler, address);
    AMQP::TcpChannel channel(&connection);

    channel.onError([](const char* message) {
        std::cerr << "RabbitMQ channel error: " << message << "\n";
    });

    // Declare topology: the command queue and the event exchange.
    channel.declareQueue(impl_->config.command_queue, AMQP::durable);
    channel.declareExchange(impl_->config.event_exchange, AMQP::fanout, AMQP::durable);

    // Subscribe the consumer to the command queue.
    channel.consume(impl_->config.command_queue)
        .onReceived([this, &channel](const AMQP::Message& message,
                                     uint64_t delivery_tag,
                                     bool /*redelivered*/) {
            const std::string payload(message.body(), message.bodySize());
            try {
                // Transport endpoint invokes the use case and returns the event JSON.
                const std::string event_json = impl_->consumer->Consume(payload);

                // Publish the resulting event.
                channel.publish(impl_->config.event_exchange, "", event_json);

                // Acknowledge successful processing.
                channel.ack(delivery_tag);
            } catch (const std::exception& ex) {
                std::cerr << "Rejecting message: " << ex.what() << "\n";
                // Reject and dead-letter (no requeue) - mirrors retry/DLQ semantics.
                channel.reject(delivery_tag);
            }
        })
        .onError([](const char* message) {
            std::cerr << "RabbitMQ consume error: " << message << "\n";
        });

    std::cout << "Payment RabbitMQ host connected to "
              << impl_->config.host << ":" << impl_->config.port << "\n";
    std::cout << "  Consuming queue:    " << impl_->config.command_queue << "\n";
    std::cout << "  Publishing exchange: " << impl_->config.event_exchange << "\n";

    // Run the event loop (blocks until Stop() or connection loss).
    ev_run(impl_->loop, 0);
    return true;
}

void RabbitMqHost::Stop() {
    if (impl_->loop) {
        ev_break(impl_->loop, EVBREAK_ALL);
    }
}

} // namespace logos::payment::service::rabbitmq_host::configuration
