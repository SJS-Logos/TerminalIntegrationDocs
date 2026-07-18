#include "logos_payment_service_rabbitmq_host/configuration/rabbitmq_configuration.h"
#include "logos_payment_service_rabbitmq_host/configuration/rabbitmq_host.h"
#include "logos_payment_service_rabbitmq_host/consumers/authorize_payment_consumer.h"

#include "logos_payment_service_core/application/container/service_container.h"
#include "logos_payment_service_core/application/use_cases/authorize_payment_use_case.h"
#include "logos_payment_service_core/domain/services/payment_authorization_service.h"
#include "logos_payment_service_core/shared_kernel/money.h"
#include "logos_payment_service_infrastructure/in_memory_payment_repository.h"
#include "logos_payment_service_infrastructure/simple_fraud_detection_service.h"

#include <iostream>
#include <memory>

using namespace logos::payment::service;

int main() {
    // Create service container.
    core::application::container::ServiceContainer container;

    // Register infrastructure (capability implementations).
    container.Register<core::capabilities::IPaymentRepository>(
        std::make_unique<infrastructure::InMemoryPaymentRepository>());

    container.Register<core::capabilities::IFraudDetectionService>(
        std::make_unique<infrastructure::SimpleFraudDetectionService>());

    // Register domain service with configuration.
    auto maximum_amount = core::shared_kernel::Money::FromMinorUnits(1000000, "USD");  // $10,000.00
    container.Register<core::domain::services::PaymentAuthorizationService>(
        std::make_unique<core::domain::services::PaymentAuthorizationService>(
            container.Resolve<core::capabilities::IFraudDetectionService>(),
            maximum_amount));

    // Create use case with resolved dependencies.
    core::application::use_cases::AuthorizePaymentUseCase authorize_use_case(
        container.Resolve<core::domain::services::PaymentAuthorizationService>(),
        container.Resolve<core::capabilities::IPaymentRepository>());

    // Create the transport endpoint (consumer).
    rabbitmq_host::consumers::AuthorizePaymentConsumer consumer(&authorize_use_case);

    // Load broker configuration (mechanical wiring) and start the host.
    auto config = rabbitmq_host::configuration::RabbitMqConfiguration::FromEnvironment();
    rabbitmq_host::configuration::RabbitMqHost host(config, &consumer);

    std::cout << "Starting Payment RabbitMQ host...\n";
    if (!host.Run()) {
        std::cerr << "Failed to start RabbitMQ host\n";
        return 1;
    }

    return 0;
}
